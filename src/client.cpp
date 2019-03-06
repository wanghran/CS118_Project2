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

#include "Packet.hpp"
#include "Conn.hpp"
#include "utils.hpp"
//using namespace std;

using std::cout;
using std::endl;
using std::setfill;
using std::setw;
using std::hex;
using std::cerr;
using std::ifstream;
using std::ios;
using std::vector;
using std::shared_ptr;
using std::string;

#define SYN_ACK 6
#define FIN_ACK 5
#define ACK 4
#define SYN 2
#define FIN 1

#define READ_DATA_BUFFER_SIZE 511
#define DATA_BUFFER_SIZE 512
#define TOTAL_BUFFER_SIZE 524
#define CWND 512
#define SS_THRESH 10000
#define MAX_ACK_SEQ 102400


struct addrinfo hints, *infoptr;


class ClientData {
public:
    vector<shared_ptr<Packet>> packets;
    int left = 0;
    int right = 0;
};


void init_connection(int argc, char* argv[], int &port, Conn &conn,
                     string &file_name);
Packet syn(Conn &conn);
void fin(Conn &conn);
ClientData gen_client_data(const string &file_name, Packet &syn_ack);
bool done(const ClientData &client_data);
void send_as_many_packets_as_possible(ClientData &client_data, const Conn &conn);
void printInt_32(uint32_t x);
void printInt_16(uint16_t x);


int main(int argc, char* argv[]){
    int port;
    Conn conn;
    string file_name;
    init_connection(argc, argv, port, conn, file_name);
    
    
    cout << "Client " << port << endl;
    Packet syn_ack = syn(conn);
    
    
    ClientData client_data = gen_client_data(file_name, syn_ack);
    
    while(!done(client_data)) {
        
        send_as_many_packets_as_possible(client_data, conn);
        
        
        //        if (sendto(clientSocket,pack.total_data,bytes_send+12,0,(struct sockaddr *)&serverAddr,addr_size) < 0) {
        //            perror("send to");
        //            exit(EXIT_FAILURE);
        //        }
        //
        //
        //        if (bytes_send == 0){
        //            cout << "done with sending file" << endl;
        //            break;
        //        }
        
        /*Receive ack from server*/
        
        // regular data ack;
        //    char ACK_buffer[TOTAL_BUFFER_SIZE];
        //    int nBytes_ACK = recvfrom(clientSocket,ACK_buffer,sizeof(ACK_buffer),0,(struct sockaddr *)&serverAddr,&addr_size);
        //    packet ACK_pack(ACK_buffer);
        
        
        
    }
    
    //    input.close();
//    fin(conn);
    close(conn.clientSocket);
    return 0;
}

void init_connection(int argc, char* argv[], int &port, Conn &conn,
                     string &file_name) {
    //  check num of args
    if(argc != 4){
        cerr << "ERROR: Number of arguments incorrect. Try ./client <HOSTNAME/IP> <PORT> <FILENAME>\n";
        exit(EXIT_FAILURE);
    }
    
    char* host_name = argv[1];
    port = atoi(argv[2]);
    file_name = argv[3];
    
    //  check ports
    if(port < 1024 || port > 65535){
        cerr << "ERROR: Port numbers incorrect. Try another one.\n";
        exit(EXIT_FAILURE);
    }
    
    //  check hostname and change hostname to ip
    memset(&hints,0,sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_INET; // AF_INET means IPv4 only addresses
    int result = getaddrinfo(host_name, NULL, &hints, &infoptr);
    if (result) {
        fprintf(stderr, "ERROR: incorrect hostname: %s\n", gai_strerror(result));
        exit(1);
    }
    struct addrinfo *p;
    char host[256];
    for (p = infoptr; p != NULL; p = p->ai_next) {
        getnameinfo(p->ai_addr, p->ai_addrlen, host, sizeof (host), NULL, 0, NI_NUMERICHOST);
    }
    freeaddrinfo(infoptr);
    //  puts(host);
    //  (above) check hostname and change hostname to ip
    
    /*Create UDP socket*/
    conn.clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (conn.clientSocket< 0) {
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


ClientData gen_client_data(const string &file_name, Packet &syn_ack) {
    char buffer[READ_DATA_BUFFER_SIZE]; //the 512th byte in the buffer is set to be \0, only read 511
    ifstream input(file_name, ios::binary);
    if(!input.is_open()){
        cerr << "ERROR: cannot open the file" << endl;
        exit(EXIT_FAILURE);
    }
    ClientData rtn;
    while(1) {
        memset(buffer, '\0', sizeof(buffer));
        int bytes_send = input.read(buffer, sizeof(buffer)).gcount();
        if (bytes_send == 0) {
            break;
        }
        cout << "Packet contains " << bytes_send << " bytes of data" << endl;
        rtn.packets.push_back(shared_ptr<Packet>(new Packet(buffer, bytes_send, 12346, 4321, Header::give_id(syn_ack.header), 4))); // TODO: properly set the nums
    }
    input.close();
    cout << "Created " << rtn.packets.size() << " packets" << endl;
    return rtn;
}

void send_as_many_packets_as_possible(ClientData &client_data, const Conn &conn) {
    int cnt = 0;
    for (auto const& packet_ptr : client_data.packets) {
        packet_ptr->send_packet(conn);
        cnt += 1;
    }
    cout << "Sent " << cnt << " packets" << endl;
    
    //    if (sendto(clientSocket,pack.total_data,bytes_send+12,0,(struct sockaddr *)&serverAddr,addr_size) < 0) {
    //        perror("send to");
    //        exit(EXIT_FAILURE);
    //    }
    //
    //
    //    if (bytes_send == 0){
    //        cout << "done with sending file" << endl;
    //        break;
    //    }
}


void recv_acks(ClientData &client_data, Conn &conn) {
    char buffer[TOTAL_BUFFER_SIZE];
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(conn.clientSocket, &readfds);
    while(true) {
        memset(buffer, '\0', sizeof(buffer));
        if (select(conn.clientSocket + 1, &readfds, NULL, NULL, NULL) > 0) {
            int nBytes = int(recvfrom(conn.clientSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&conn.addr, &conn.addr_size));
            Packet recv_pack(buffer, nBytes);
            printf("server received %d bytes\n", nBytes);
            cout << "recv_pack.header.seq_num " << ntohl(recv_pack.header.seq_num) << endl;
            cout << "recv_pack.header.ack_num " << ntohl(recv_pack.header.ack_num) << endl;
            cout << "recv_pack.header.ID " << ntohs(recv_pack.header.ID) << endl;
            cout << "recv_pack.header.flag " << ntohs(recv_pack.header.flag) << endl;
            cout << "recv_pack.data " << recv_pack.data << "\n" << endl;
            int packet_id = client_convert_to_packet_id(ntohl(recv_pack.header.seq_num), nBytes);
            assert (packet_id >= 0 and packet_id < client_data.packets.size());
            client_data.packets[packet_id]->state = ACKED;
            // TODO: modify L and R pointers
        } else {
            break;
        }
    }
}


Packet syn(Conn &conn) {
    char SYN_buffer[1];
    memset(SYN_buffer, '\0', sizeof(SYN_buffer));
    Packet SYN_pack(SYN_buffer, DATA_BUFFER_SIZE, 12345, 0, 0, SYN);
    SYN_pack.send_packet(conn);
    while(1){
        char SYN_ACK_buffer[TOTAL_BUFFER_SIZE];
        cout << "Waiting to receive from server..." << endl;
        int nBytes_SYN_ACK = recvfrom(conn.clientSocket,SYN_ACK_buffer,sizeof(SYN_ACK_buffer),0,(struct sockaddr *)&conn.addr,&conn.addr_size);
        Packet SYN_ACK_pack(SYN_ACK_buffer, nBytes_SYN_ACK);
        cout << "received byte " << nBytes_SYN_ACK << endl;
        cout << "recv_pack.header.seq_num " << ntohl(SYN_ACK_pack.header.seq_num) << endl;
        cout << "recv_pack.header.ack_num " << ntohl(SYN_ACK_pack.header.ack_num) << endl;
        cout << "recv_pack.header.ID " << ntohs(SYN_ACK_pack.header.ID) << endl;
        cout << "recv_pack.header.flag " << ntohs(SYN_ACK_pack.header.flag) << endl;
        if (ntohs(SYN_ACK_pack.header.flag) == 6){
           return SYN_ACK_pack;
        }
        else{
            continue;
        }
        
    }
    
}

//needs to change
void fin(Conn &conn) {
    char SYN_buffer[1];
    memset(SYN_buffer, '\0', sizeof(SYN_buffer));
    Packet SYN_pack(SYN_buffer, DATA_BUFFER_SIZE, 12345, 0, 0, SYN);
    SYN_pack.send_packet(conn);
    while(1){
        char SYN_ACK_buffer[TOTAL_BUFFER_SIZE];
        int nBytes_SYN_ACK = recvfrom(conn.clientSocket,SYN_ACK_buffer,sizeof(SYN_ACK_buffer),0,(struct sockaddr *)&conn.addr,&conn.addr_size);
        Packet SYN_ACK_pack(SYN_ACK_buffer, nBytes_SYN_ACK);
        cout << "received byte " << nBytes_SYN_ACK << endl;
        cout << "recv_pack.header.seq_num " << ntohl(SYN_ACK_pack.header.seq_num) << endl;
        cout << "recv_pack.header.ack_num " << ntohl(SYN_ACK_pack.header.ack_num) << endl;
        cout << "recv_pack.header.ID " << ntohs(SYN_ACK_pack.header.ID) << endl;
        cout << "recv_pack.header.flag " << ntohs(SYN_ACK_pack.header.flag) << endl;
        if (ntohs(SYN_ACK_pack.header.flag) == 6){
            break;
        }
        else{
            continue;
        }
        
    }
    
}




bool done(const ClientData &client_data) {
    for (auto const& packet_ptr : client_data.packets) {
        if (packet_ptr->state != SENT) { // TODO: change SENT to ACKED
            return false;
        }
    }
    return true;
}


void printInt_32(uint32_t x)
{
    cout << setfill('0') << setw(8) << hex << x << '\n';
}

void printInt_16(uint16_t x)
{
    cout << setfill('0') << setw(4) << hex << x << '\n';
}



