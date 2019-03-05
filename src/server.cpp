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

#include "Packet.hpp"

using std::cout;
using std::endl;
using std::cerr;
using std::ofstream;
using std::ios;



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
    fd_set rset;
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
  //Non-blocking UDP
  fcntl(udpSocket, F_SETFL, O_NONBLOCK);

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

  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(udpSocket, &readfds);

  while(1){
    memset(buffer, '\0', sizeof(buffer));
    if (select(udpSocket + 1, &readfds, NULL, NULL, NULL) > 0) {
      nBytes = recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &addr_size);

      if (nBytes == -1){
          cerr << "ERROR: byte receive error";
          output.close();
          close(udpSocket);
      }


      if (nBytes == 12){
          break;
      }

      // cout << buffer << endl;
      Packet recv_pack(buffer);

      printf("server received %d bytes\n", nBytes);
      cout << "recv_pack.header.seq_num " << ntohl(recv_pack.header.seq_num) << endl;
      cout << "recv_pack.header.ack_num " << ntohl(recv_pack.header.ack_num) << endl;
      cout << "recv_pack.header.ID " << ntohs(recv_pack.header.ID) << endl;
      cout << "recv_pack.header.flag " << ntohs(recv_pack.header.flag) << endl;
      cout << "recv_pack.data " << recv_pack.data << "\n" <<endl;


      if (Header::give_flag(recv_pack.header) == SYN){
          char local_buffer[DATA_BUFFER_SIZE - 1];
          memset(local_buffer, '\0', sizeof(local_buffer));
          unsigned int ack_reply =  ntohl(recv_pack.header.seq_num) + 1;
          Packet pack(local_buffer, DATA_BUFFER_SIZE, 4321, ack_reply, 1, SYN_ACK);  // 1 need to change based on connection id;
          sendto(udpSocket,pack.total_data,TOTAL_BUFFER_SIZE,0,(struct sockaddr *)&clientAddr,addr_size);
          continue;
      }

        // fin needs to change
      if (Header::give_flag(recv_pack.header) == FIN){
          char local_buffer[DATA_BUFFER_SIZE - 1];
          memset(local_buffer, '\0', sizeof(local_buffer));
          unsigned int ack_reply =  ntohl(recv_pack.header.seq_num) + 1;
          Packet pack(local_buffer, DATA_BUFFER_SIZE, 4322, ack_reply, 1, FIN_ACK);  // 1 need to change based on connection id;
          sendto(udpSocket,pack.total_data,TOTAL_BUFFER_SIZE,0,(struct sockaddr *)&clientAddr,addr_size);
          continue;
      }
      output.write(recv_pack.data, nBytes-12);
    }
  }
}


//// Server program
//#include <arpa/inet.h>
//#include <cerrno>
//#include <netinet/in.h>
//#include <csignal>
//#include <cstdio>
//#include <cstdlib>
//#include <cstring>
//#include <sys/socket.h>
//#include <sys/types.h>
//#include <unistd.h>
//#include <iostream>
////using namespace std;
//#define PORT 5000
//#define MAXLINE 1024
//int max(int x, int y)
//{
//    if (x > y)
//        return x;
//    else
//        return y;
//}
//int main()
//{
//    int listenfd, connfd, udpfd, nready, maxfdp1;
//    char buffer[MAXLINE];
//    pid_t childpid;
//    fd_set rset;
//    ssize_t n;
//    socklen_t len;
//    const int on = 1;
//    struct sockaddr_in cliaddr, servaddr;
//    char* message = "Hello Client";
//    void sig_chld(int);
////    cout << message << "###" << endl;
//
//    /* create listening TCP socket */
//    listenfd = socket(AF_INET, SOCK_STREAM, 0);
//    bzero(&servaddr, sizeof(servaddr));
//    servaddr.sin_family = AF_INET;
//    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
//    servaddr.sin_port = htons(PORT);
//
//    // binding server addr structure to listenfd
////    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
////    listen(listenfd, 10);
//
//    /* create UDP socket */
//    udpfd = socket(AF_INET, SOCK_DGRAM, 0);
//    // binding server addr structure to udp sockfd
//    bind(udpfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
//
//    // clear the descriptor set
//    FD_ZERO(&rset);
//
//    // get maxfd
//    maxfdp1 = max(listenfd, udpfd) + 1;
//    for (;;) {
//
//        // set listenfd and udpfd in readset
////        FD_SET(listenfd, &rset);
//        FD_SET(udpfd, &rset);
//
////        cout << "!!!" << endl;
//        // select the ready descriptor
//        nready = select(maxfdp1, &rset, NULL, NULL, NULL);
//        
////        cout << "***" << endl;
//
//        // if tcp socket is readable then handle
//        // it by accepting the connection
////        cout << "@@@" << endl;
//        if (FD_ISSET(listenfd, &rset)) {
//            len = sizeof(cliaddr);
//            connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len);
//            if ((childpid = fork()) == 0) {
//                close(listenfd);
//                bzero(buffer, sizeof(buffer));
////                printf("Message From TCP client: ");
//                read(connfd, buffer, sizeof(buffer));
//                puts(buffer);
//                write(connfd, (const char*)message, sizeof(buffer));
//                close(connfd);
//                exit(0);
//            }
//            close(connfd);
//        }
//        // if udp socket is readable receive the message.
//        if (FD_ISSET(udpfd, &rset)) {
//            len = sizeof(cliaddr);
//            bzero(buffer, sizeof(buffer));
////            cout <<"\nMessage from UDP client: \n";
//            printf("\nMessage from UDP client: ");
//            n = recvfrom(udpfd, buffer, sizeof(buffer), 0,
//                         (struct sockaddr*)&cliaddr, &len);
////            cout << n << endl;
//            printf("n=%d", n);
//            std::cout << "n=" << n << std::endl;
//            puts(buffer);
//            sendto(udpfd, (const char*)message, sizeof(buffer), 0,
//                   (struct sockaddr*)&cliaddr, sizeof(cliaddr));
//        } 
//    } 
//} 
