#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <vector>
#include <memory>
#include <cassert>
#include <time.h>

#include "Packet.hpp"
#include "Conn.hpp"
#include "utils.hpp"
//using namespace std;

using std::cerr;
using std::cout;
using std::endl;
using std::hex;
using std::ifstream;
using std::ios;
using std::setfill;
using std::setw;
using std::shared_ptr;
using std::string;
using std::vector;

#define READ_DATA_BUFFER_SIZE 512
#define CWND 512
#define SS_THRESH 10000
#define MAX_ACK_SEQ 102400

struct addrinfo hints, *infoptr;

class ClientData
{
  public:
    vector<shared_ptr<Packet>> packets;
    int left = 0;
    int cwnd = CWND;
    int ss_thresh = SS_THRESH;
    int current_sent = 0;
};

void init_connection(int argc, char *argv[], int &port, Conn &conn,
                     string &file_name);
shared_ptr<Packet> syn(Conn &conn);
void fin(ClientData &client_data, Conn &conn, shared_ptr<Packet> syn_ack);
ClientData gen_client_data(const string &file_name, shared_ptr<Packet> syn_ack);
bool done(const ClientData &client_data);
void send_as_many_packets_as_possible(ClientData &client_data, const Conn &conn);
void recv_acks(ClientData &client_data, Conn &conn);
void timeout_resend(ClientData &client_data, const Conn &conn);
bool congestion_control_can_send(ClientData &client_data, const shared_ptr<Packet> packet_ptr, int bytes_sent_so_far);

int main(int argc, char *argv[])
{
    int port;
    Conn conn;
    string file_name;
    init_connection(argc, argv, port, conn, file_name);

    D(cout << "Client " << port << endl;)
    shared_ptr<Packet> syn_ack = syn(conn);
    D(cout << "Syn done" << endl;)

    ClientData client_data = gen_client_data(file_name, syn_ack);

    while (!done(client_data))
    {

        send_as_many_packets_as_possible(client_data, conn);

        recv_acks(client_data, conn);

        timeout_resend(client_data, conn);
    }

    fin(client_data, conn, syn_ack);
    close(conn.socket);
    return 0;
}

void init_connection(int argc, char *argv[], int &port, Conn &conn,
                     string &file_name)
{
    //  check num of args
    if (argc != 4)
    {
        cerr << "ERROR: Number of arguments incorrect. Try ./client <HOSTNAME/IP> <PORT> <FILENAME>\n";
        exit(EXIT_FAILURE);
    }

    char *host_name = argv[1];
    port = atoi(argv[2]);
    file_name = argv[3];

    //  check ports
    if (port < 1024 || port > 65535)
    {
        cerr << "ERROR: Port numbers incorrect. Try another one.\n";
        exit(EXIT_FAILURE);
    }

    //  check hostname and change hostname to ip
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_INET; // AF_INET means IPv4 only addresses
    int result = getaddrinfo(host_name, NULL, &hints, &infoptr);
    if (result)
    {
        fprintf(stderr, "ERROR: incorrect hostname: %s\n", gai_strerror(result));
        exit(1);
    }
    struct addrinfo *p;
    char host[256];
    for (p = infoptr; p != NULL; p = p->ai_next)
    {
        getnameinfo(p->ai_addr, p->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
    }
    freeaddrinfo(infoptr);

    /*Create UDP socket*/
    conn.socket = socket(PF_INET, SOCK_DGRAM, 0);
    //Non-blocking UDP
    fcntl(conn.socket, F_SETFL, O_NONBLOCK);

    FD_ZERO(&conn.read_fds);
    FD_SET(conn.socket, &conn.read_fds);

    if (conn.socket < 0)
    {
        fprintf(stderr, "ERROR: socket creation failed\n");
        exit(1);
    }

    /*Configure settings in address struct*/
    conn.addr.sin_family = AF_INET;
    conn.addr.sin_port = htons(port);
    conn.addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(conn.addr.sin_zero, '\0', sizeof conn.addr.sin_zero);

    /*Initialize size variable to be used later on*/
    conn.addr_size = sizeof conn.addr;
}

ClientData gen_client_data(const string &file_name, shared_ptr<Packet> syn_ack)
{
    char buffer[READ_DATA_BUFFER_SIZE]; //the 512th byte in the buffer is set to be \0, only read 511
    ifstream input(file_name, ios::binary);
    if (!input.is_open())
    {
        cerr << "ERROR: cannot open the file" << endl;
        exit(EXIT_FAILURE);
    }
    ClientData rtn;
    int start_seq = CLIENT_DATA_START_SEQ_NUM;
    int cnt = 0;
    while (true)
    {
        memset(buffer, '\0', sizeof(buffer));
        int bytes_send = input.read(buffer, sizeof(buffer)).gcount();
        if (bytes_send == 0)
        {
            break;
        }
        //        D(cout << "Packet contains " << bytes_send << " bytes of data" << endl;)
        int flag = 0;
        int ack_num = 0;
        if (cnt == 0)
        {
            flag = ACK;
            ack_num = SERVER_DATA_START_SEQ_NUM;
        }
        start_seq = client_get_next_seq_num(start_seq);
        shared_ptr<Packet> new_pack(new Packet(buffer, bytes_send, start_seq, ack_num, Header::give_id(syn_ack->header), flag));
        //        new_pack->print_packet();
        rtn.packets.push_back(new_pack); // TODO: properly set the nums
        start_seq += bytes_send;
        ++cnt;
    }
    input.close();
    //    cout << "Created " << rtn.packets.size() << " packets" << endl;
    return rtn;
}

void send_as_many_packets_as_possible(ClientData &client_data, const Conn &conn)
{
    int cnt = 0;
    int bytes_sent_so_far = 0;
    client_data.current_sent = 0;
    for (int i = client_data.left; i < int(client_data.packets.size()); ++i)
    {
        auto const packet_ptr = client_data.packets[i];
        if (congestion_control_can_send(client_data, packet_ptr, bytes_sent_so_far))
        {
            if (packet_ptr->state == INIT || packet_ptr->state == TIMEOUT)
            { // if already sent, no sent again
                if (packet_ptr->state == INIT)
                {
                    //                    cout << "@@ " << packet_ptr->state << endl;
                    packet_ptr->official_send_print(true, client_data.cwnd, client_data.ss_thresh, false);
                }
                else
                {
                    //                    cout << "INIT" << INIT << SENT << ACKED << TIMEOUT << endl;
                    //                    cout << "## " << packet_ptr->state << endl;
                    packet_ptr->official_send_print(true, client_data.cwnd, client_data.ss_thresh, true);
                }
                packet_ptr->send_packet(conn);
                ++client_data.current_sent;
                D(cout << ">>> Sent packet " << i << " packet content:" << endl;)
                packet_ptr->print_packet();
                cnt += 1;
                bytes_sent_so_far += (packet_ptr->data_bytes);
            }
        }
        else
        {
            break;
        }
    }
    if (cnt > 0)
    {
        D(cout << "Sent " << cnt << " packets" << endl;)
    }
}

void recv_acks(ClientData &client_data, Conn &conn)
{
    assert(client_data.current_sent >= 0);
    D(cout << "### client_data.current_sent " << client_data.current_sent << endl;)
    for (int i = 0; i < client_data.current_sent; ++i)
    {
        shared_ptr<Packet> recv_pack_ptr = recv_packet(conn);
        if (recv_pack_ptr)
        {
            recv_pack_ptr->official_recv_print(true, client_data.cwnd, client_data.ss_thresh);
            int n_bytes = recv_pack_ptr->total_bytes;
            D(cout << "<<< Received ack, packet content:" << endl;)
            recv_pack_ptr->print_packet();
            int packet_id = client_convert_to_packet_id(Header::give_ack(recv_pack_ptr->header), n_bytes);
            D(cout << "<<< Packet " << packet_id << " acked" << endl;)
            assert(packet_id >= 0 and packet_id < client_data.packets.size());
            client_data.packets[packet_id]->state = ACKED;
            //            cout << "@@@ Header::give_ack(recv_pack_ptr->header) " << Header::give_ack(recv_pack_ptr->header) << " packet_id " << packet_id << endl;
            if (client_data.left < client_data.packets.size() - 1)
            {
                client_data.left = packet_id + 1;
                //                assert (client_data.left >= 0 and client_data.left < client_data.packets.size());
            }
            if (client_data.cwnd < client_data.ss_thresh)
            {
                client_data.cwnd += CWND;
            }
            else
            {
                client_data.cwnd += (CWND * CWND) / client_data.cwnd;
            }
        }
        else
        {
            D(cout << "0.5 secs has passed --> all packets that have been sent are timeout" << endl;)
        }
    }
}

void timeout_resend(ClientData &client_data, const Conn &conn)
{
    int cnt = 0;
    for (int i = client_data.left; i < int(client_data.packets.size()); ++i)
    {
        auto const packet_ptr = client_data.packets[i];
        if (packet_ptr->is_timeout())
        {
            D(cout << "packet timeout!" << endl;)
            client_data.ss_thresh = client_data.cwnd / 2;
            client_data.cwnd = CWND;
            packet_ptr->state = TIMEOUT;
            // for send_as_many_packets_as_possible to resend
            //            packet_ptr->send_packet(conn); // resend
            cnt += 1;
            break; //
        }
    }
    if (cnt > 0)
    {
        D(cout << "Resent " << cnt << " packets" << endl;)
    }
}

bool congestion_control_can_send(ClientData &client_data, const shared_ptr<Packet> packet_ptr, int bytes_sent_so_far)
{
    int total_would_be = bytes_sent_so_far + packet_ptr->data_bytes;
    D(cout << "----- total_would_be " << total_would_be << " client_data.cwnd " << client_data.cwnd << endl;)
    return total_would_be <= client_data.cwnd;
}

shared_ptr<Packet> syn(Conn &conn)
{
    char SYN_buffer[1];
    memset(SYN_buffer, '\0', sizeof(SYN_buffer));
    Packet SYN_pack(SYN_buffer, 1, 12345, 0, 0, SYN);
    while (true)
    {
        if (SYN_pack.state == INIT)
        {
            D(cout << "Syn: Init sending" << endl;)
            SYN_pack.send_packet(conn);
            SYN_pack.official_send_print(true, CWND, SS_THRESH, false);
        }
        if (SYN_pack.is_timeout())
        {
            D(cout << "Syn: Resending" << endl;)
            SYN_pack.send_packet(conn);
            SYN_pack.official_send_print(true, CWND, SS_THRESH, false);
        }
        shared_ptr<Packet> recv_pack_ptr = recv_packet(conn);
        if (recv_pack_ptr)
        {
            u_int32_t ack_num = Header::give_ack(recv_pack_ptr->header);
            if (ack_num != 12346)
            {
                recv_pack_ptr->official_drop_print();
                recv_pack_ptr->print_packet();
            }
            else
            {
                recv_pack_ptr->official_recv_print(true, CWND, SS_THRESH);
                recv_pack_ptr->print_packet();
                return recv_pack_ptr;
            }
            // assert (ntohs(recv_pack_ptr->header.flag) == 6);
        }
        else
        {
            continue; // keep sending the syn packet until receiving ack
        }
    }
}


void fin(ClientData &client_data, Conn &conn, shared_ptr<Packet> syn_ack)
{
    while (true)
    {
        char FIN_buffer[1];
        memset(FIN_buffer, '\0', sizeof(FIN_buffer));
        assert(!client_data.packets.empty());
        int seq_num = Header::give_seq(client_data.packets.back()->header) + 1 + client_data.packets.back()->data_bytes;
        D(cout << "@@@ seq_num " << seq_num << endl;)
        client_data.packets.back()->print_packet();
        Packet FIN_pack1(FIN_buffer, 1, seq_num, 0, Header::give_id(syn_ack->header), FIN);
        while (true)
        {
            D(cout << "Fin: pack 1" << endl;)
            FIN_pack1.print_packet();
            FIN_pack1.send_packet(conn);
            FIN_pack1.official_send_print(true, client_data.cwnd, client_data.ss_thresh, false);

            shared_ptr<Packet> recv_pack_ptr = recv_packet(conn);
            D(cout << "recv or not" << endl;)
            if (recv_pack_ptr)
            {
                if (Header::give_id(recv_pack_ptr->header) != Header::give_id(syn_ack->header))
                {
                    recv_pack_ptr->official_drop_print();
                }
                else if (Header::give_ack(recv_pack_ptr->header) != seq_num + 1)
                {
                    recv_pack_ptr->official_drop_print();
                }
                else
                {
                    clock_t start = clock();
                    recv_pack_ptr->official_recv_print(true, client_data.cwnd, client_data.ss_thresh);
                    D(cout << ":)" << endl;)
                    D(cout << "Fin: pack 1 acked" << endl;)
                    recv_pack_ptr->print_packet();
                    //                assert (ntohs(recv_pack_ptr->header.flag) == ACK or ntohs(recv_pack_ptr->header.flag) == FIN); // check
                    Packet FIN_pack2(FIN_buffer, 1, seq_num + 1 + DATA_SIZE, SERVER_DATA_START_SEQ_NUM + 1, Header::give_id(syn_ack->header), ACK); // TODO: Kim's server does not close
                    D(cout << "Fin: pack 2" << endl;)
                    FIN_pack2.print_packet();
                    FIN_pack2.send_packet(conn);
                    FIN_pack2.official_send_print(true, client_data.cwnd, client_data.ss_thresh, false);
                    while ((clock() - start) / 60 <= 2)
                    {
                        shared_ptr<Packet> recv_pack_aft_ptr = recv_packet(conn);
                        if (recv_pack_aft_ptr)
                        {
                            recv_pack_aft_ptr->official_drop_print();
                        }
                        else
                        {
                            continue;
                        }
                    }
                    return;
                }
            }
            else
            {
                D(cout << ":(" << endl;)
                //                exit(-1);
                continue; // keep sending the syn packet until receiving ack
            }
        }
    }
}

bool done(const ClientData &client_data)
{
    return client_data.packets.back()->state == ACKED;
}
