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

#include "packet.hpp"

using namespace std;


#define SYN_ACK 6
#define FIN_ACK 5
#define ACK 4
#define SYN 2
#define FIN 1

#define DATA_BUFFER_SIZE 512
#define TOTAL_BUFFER_SIZE 524
#define CWND 512
#define SS_THRESH 10000
#define MAX_ACK_SEQ 102400



int thread_id = 0;
static const int num_threads = 11;
int cgwn_size = 1; //should be 512, will change later

void sig_quit_handler(int s)
{
  cout << "Interrupt signal (SIGQUIT) received.\n";
  exit(0);
}

void sig_term_handler(int s)
{
  cout << "Interrupt signal (SIGTERM) received.\n";
  exit(0);
}



int main(int argc, char* argv[]){

    signal(SIGQUIT, sig_quit_handler);
    signal(SIGTERM, sig_term_handler);

    if (argc != 3) {
        cerr << "ERROR: Number of arguments incorrect. Try ./server <PORT> <FILE-DIR>\n";
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    char *save_directory = argv[2];

    struct stat st = {0};
    if (stat(save_directory, &st) == -1) {
        mkdir(save_directory, 0700);
    }

    if (port < 1024 || port > 65535) {
        cerr << "ERROR: Port numbers incorrect. Try another one.\n";
        exit(EXIT_FAILURE);
    }



  int udpSocket, nBytes;
  char buffer[TOTAL_BUFFER_SIZE];
  struct sockaddr_in serverAddr, clientAddr;
//  struct sockaddr_storage serverStorage;
  socklen_t addr_size, client_addr_size;

  /*Create UDP socket*/
  udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

  /*Configure settings in address struct*/
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  /*Bind socket with address struct*/
  bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

  /*Initialize size variable to be used later on*/
  addr_size = sizeof clientAddr;


// save to file
  char file_name[30];
  sprintf(file_name,"%s/%d.file", save_directory, 1);

  ofstream output(file_name, ios::out | ios::trunc | ios::binary);


  while(1){
    memset(buffer, '\0', sizeof(buffer));
    nBytes = recvfrom(udpSocket,buffer,sizeof(buffer),0,(struct sockaddr *)&clientAddr, &addr_size);

    if (nBytes == -1){
        cerr << "ERROR: byte receive error";
        output.close();
        close(udpSocket);
    }


    if (nBytes == 12){
        break;
    }

    // cout << buffer << endl;
    packet recv_pack(buffer);
  
    printf("server received %d bytes\n", nBytes);
    cout << "recv_pack.header.seq_num " << ntohl(recv_pack.header.seq_num) << endl;
    cout << "recv_pack.header.ack_num " << ntohl(recv_pack.header.ack_num) << endl;
    cout << "recv_pack.header.ID " << ntohs(recv_pack.header.ID) << endl;
    cout << "recv_pack.header.flag " << ntohs(recv_pack.header.flag) << endl;
    cout << "recv_pack.data " << recv_pack.data << "\n" <<endl;


    if (ntohs(recv_pack.header.flag) == SYN){
        char local_buffer[DATA_BUFFER_SIZE - 1];
        memset(local_buffer, '\0', sizeof(local_buffer));
        unsigned int ack_reply =  ntohl(recv_pack.header.seq_num) + 1;
        packet pack(local_buffer, DATA_BUFFER_SIZE, 4321, ack_reply, 1, SYN_ACK);  // 1 need to change based on connection id;
        sendto(udpSocket,pack.total_data,TOTAL_BUFFER_SIZE,0,(struct sockaddr *)&clientAddr,addr_size);
        continue;
    }

     // fin needs to change
    if (ntohs(recv_pack.header.flag) == FIN){
        char local_buffer[DATA_BUFFER_SIZE - 1];
        memset(local_buffer, '\0', sizeof(local_buffer));
        unsigned int ack_reply =  ntohl(recv_pack.header.seq_num) + 1;
        packet pack(local_buffer, DATA_BUFFER_SIZE, 4322, ack_reply, 1, FIN_ACK);  // 1 need to change based on connection id;
        sendto(udpSocket,pack.total_data,TOTAL_BUFFER_SIZE,0,(struct sockaddr *)&clientAddr,addr_size);
        continue;
    }
// normal packet received

    output.write(recv_pack.data, nBytes-12);


//    sendto(udpSocket,buffer,nBytes,0,(struct sockaddr *)&serverStorage,addr_size);
  }

 output.close();
//  close(udpSocket);
  return 0;
}