#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <thread>
#include <cstring>
#include <deque>
#include <memory>
#include <map>

#include "Packet.hpp"
#include "utils.hpp"

using std::cerr;
using std::cout;
using std::deque;
using std::endl;
using std::ios;
using std::map;
using std::ofstream;
using std::shared_ptr;

#define SYN_ACK 6
#define FIN_ACK 5
#define ACK 4
#define SYN 2
#define FIN 1
#define NO_FLAG 0

#define DATA_BUFFER_SIZE 512
#define TOTAL_BUFFER_SIZE 524
#define CWND 512
#define SS_THRESH 10000
#define MAX_ACK_SEQ 102400

struct client_stats
{
    int seq_num = 4322;
    int last_legit_ack_num = 12346;
    deque<shared_ptr<Packet>> packet_ptr_buffer;
    struct sockaddr_in client_address;
    socklen_t client_addr_size;
    char client_file[30];
};

int cgwn_size = 1; //should be 512, will change later
// create client id: client stats mapping
map<int, client_stats> clients_map;

void sig_quit_handler(int s)
{
    cerr << "Interrupt signal (SIGQUIT) received.\n";
    exit(0);
}

void sig_term_handler(int s)
{
    cerr << "Interrupt signal (SIGTERM) received.\n";
    exit(0);
}

void syn_handler(int udpSocket, sockaddr_in clientAddr, socklen_t addr_size, shared_ptr<Packet> recv_pack, char *save_directory);
void normal_packet_handler(int udpSocket, sockaddr_in clientAddr, socklen_t addr_size, shared_ptr<Packet> recv_pack, int nBytes);
void fin_handler(int udpSocket, sockaddr_in clientAddr, socklen_t addr_size, shared_ptr<Packet> recv_pack);

int main(int argc, char *argv[])
{
    fd_set rset;
    signal(SIGQUIT, sig_quit_handler);
    signal(SIGTERM, sig_term_handler);

    if (argc != 3)
    {
        cerr << "ERROR: Number of arguments incorrect. Try ./server <PORT> <FILE-DIR>\n";
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    char *save_directory = argv[2];

    struct stat st = {0};
    if (stat(save_directory, &st) == -1)
    {
        mkdir(save_directory, 0700);
    }

    if (port < 1024 || port > 65535)
    {
        cerr << "ERROR: Port numbers incorrect. Try another one.\n";
        exit(EXIT_FAILURE);
    }

    int udpSocket, nBytes;
    char buffer[TOTAL_BUFFER_SIZE];
    struct sockaddr_in serverAddr, clientAddr;
    //struct sockaddr_storage serverStorage;
    socklen_t addr_size, client_addr_size;

    /*Create UDP socket*/
    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);
    //Non-blocking UDP
    fcntl(udpSocket, F_SETFL, O_NONBLOCK);

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    /*Bind socket with address struct*/
    bind(udpSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    /*Initialize size variable to be used later on*/
    addr_size = sizeof clientAddr;

    // save to file

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(udpSocket, &readfds);

    while (1)
    {

        memset(buffer, '\0', sizeof(buffer));

        if (select(udpSocket + 1, &readfds, NULL, NULL, NULL) > 0)
        {
            nBytes = recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &addr_size);

            shared_ptr<Packet> recv_pack(new Packet(buffer, nBytes));

            printf("server received %d bytes\n", nBytes);
            cout << "recv_pack.header.seq_num " << ntohl(recv_pack->header.seq_num) << endl;
            cout << "recv_pack.header.ack_num " << ntohl(recv_pack->header.ack_num) << endl;
            cout << "recv_pack.header.ID " << ntohs(recv_pack->header.ID) << endl;
            cout << "recv_pack.header.flag " << ntohs(recv_pack->header.flag) << endl;
            cout << "recv_pack.data " << recv_pack->data << "\n"
                 << endl;

            if (Header::give_flag(recv_pack->header) == SYN)
            {
                syn_handler(udpSocket, clientAddr, addr_size, recv_pack, save_directory);
                continue;
            }
            else if (Header::give_flag(recv_pack->header) == NO_FLAG)
            {
                normal_packet_handler(udpSocket, clientAddr, addr_size, recv_pack, nBytes);
                continue;
            }
            // fin needs to change
            else if (Header::give_flag(recv_pack->header) == FIN)
            {
                u_int16_t id = Header::give_id(recv_pack->header);
                client_stats stats = clients_map[id];
                fstream output(stats.client_file, std::ios::out | std::ios::binary);
                for (shared_ptr<Packet> packet_ptr : stats.packet_ptr_buffer)
                {
                    // cout << "data byte " << packet_ptr->data_bytes << endl;
                    output.write(packet_ptr->data, packet_ptr->data_bytes);
                }
                output.close();
                fin_handler(udpSocket, clientAddr, addr_size, recv_pack);
                continue;
            }

        }
    }
}

void syn_handler(int udpSocket, sockaddr_in clientAddr, socklen_t addr_size, shared_ptr<Packet> recv_pack, char *save_directory)
{

    int id = clients_map.size() + 1;
    client_stats c_stats;
    char file_name[30];
    sprintf(file_name, "%s/%d.file", save_directory, id);

    strncpy(c_stats.client_file, file_name, 30);
    c_stats.client_address = clientAddr;
    c_stats.client_addr_size = addr_size;

    char local_buffer[1]{'\0'};
    unsigned int ack_reply = Header::give_seq(recv_pack->header) + 1;
    Packet pack(local_buffer, 1, 4321, ack_reply, id, SYN_ACK);

    // update share ptr deck
    // update seq number
    // update legit

    clients_map[id] = c_stats;
    sendto(udpSocket, pack.total_data, TOTAL_BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, addr_size);
}

void normal_packet_handler(int udpSocket, sockaddr_in clientAddr, socklen_t addr_size, shared_ptr<Packet> recv_pack, int nBytes)
{

    u_int16_t id = Header::give_id(recv_pack->header);
    u_int32_t seq_num = Header::give_seq(recv_pack->header);
    client_stats &c_stats = clients_map[id];
    int packet_num = server_convert_to_packet_id(Header::give_seq(recv_pack->header) + nBytes - 12);
    //mod
    cout << "packet number " << packet_num << endl;
    if (c_stats.packet_ptr_buffer.size() < packet_num)
    {
        for (int i = 0; i < packet_num - c_stats.packet_ptr_buffer.size(); i++)
        {
            c_stats.packet_ptr_buffer.push_back(nullptr);
        }
        c_stats.packet_ptr_buffer[packet_num - 1] = recv_pack;
    }

    cout << c_stats.client_file << endl;

    if (Header::give_seq(recv_pack->header) == c_stats.last_legit_ack_num)
    {
        cout << "c_stats.last_legit_ack_num" << c_stats.last_legit_ack_num << endl;
        int j = server_convert_to_packet_id(Header::give_seq(recv_pack->header) + nBytes - 12);

        cout << "j is : " << j << endl;
        while (j < c_stats.packet_ptr_buffer.size()){
             if (c_stats.packet_ptr_buffer[j] == nullptr){
                 break;
             }
            j++;
        }
        
       c_stats.last_legit_ack_num = server_convert_to_byte_num(j-1) + recv_pack->data_bytes;
       cout << "c_stats.last_legit_ack_num" << c_stats.last_legit_ack_num << endl;
    }

    char local_buffer[DATA_BUFFER_SIZE]{'\0'}; //data buffer size is correct?
    Packet pack(local_buffer, 1, 4322, c_stats.last_legit_ack_num, id, ACK);
    sendto(udpSocket, pack.total_data, TOTAL_BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, addr_size);




    // if all fin, write to file
    // ofstream output(c_stats.client_file, ios::out | ios::app | ios::binary);
    if (nBytes == -1)
    {
        cerr << "ERROR: byte receive error";
        // output.close();
        close(udpSocket);
    }
    if (nBytes == 12)
    {
        // output.close();
        close(udpSocket);
        //          break;

    }

    // output.write(recv_pack.data, nBytes - 12);



}

void fin_handler(int udpSocket, sockaddr_in clientAddr, socklen_t addr_size, shared_ptr<Packet> recv_pack)
{

    char local_buffer[1]{'\0'};
    u_int32_t ack_reply = Header::give_seq(recv_pack->header) + 1;
    u_int16_t id = Header::give_id(recv_pack->header);
    Packet pack(local_buffer, 1, 4322, ack_reply, id, FIN_ACK); 

    sendto(udpSocket, pack.total_data, TOTAL_BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, addr_size);

    //    ofstream output(file_name, ios::out | ios::trunc | ios::binary);

}
