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
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <deque>

#include "packet.hpp"

//http://developerweb.net/viewtopic.php?id=3196

using namespace std;
#define DATA_BUFFER_SIZE 512
#define TOTAL_BUFFER_SIZE 524
struct addrinfo hints, *infoptr;

int SS_THRESH = 10000;
int cgwn_size = 1; //should be 512, will change later 

int main(int argc, char* argv[]){

//  check num of args
  if(argc != 4){
    cerr << "ERROR: Number of arguments incorrect. Try ./client <HOSTNAME/IP> <PORT> <FILENAME>\n";
    exit(EXIT_FAILURE);
  }

  char* host_name = argv[1];
  int port = atoi(argv[2]);
  char* file_name = argv[3];

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
  puts(host);
 //  (above) check hostname and change hostname to ip



  int clientSocket, portNum, nBytes;
  char buffer[DATA_BUFFER_SIZE - 1]; //the 512th byte in the buffer is set to be \0, only read 511
  struct sockaddr_in serverAddr;
  socklen_t addr_size;

  /*Create UDP socket*/
  clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

  /*Configure settings in address struct*/
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  /*Initialize size variable to be used later on*/
  addr_size = sizeof serverAddr;

  ifstream input (file_name, ios::binary);
  if(!input.is_open()){
      cerr << "ERROR: cannot open the file" << endl;
      exit(EXIT_FAILURE);
  }



  while(packet_num < cgwn_size){
    memset(buffer, '\0', sizeof(buffer));
    int bytes_send = input.read(buffer, sizeof(buffer)).gcount();
    cout << "byte send: " << bytes_send << endl;
    packet pack(buffer, DATA_BUFFER_SIZE, 12345, 4321, 1, 4);


    if (sendto(clientSocket,pack.total_data,bytes_send+12,0,(struct sockaddr *)&serverAddr,addr_size) < 0) {
        perror("send to");
        exit(EXIT_FAILURE);
    }

    if (bytes_send == 0){
        cout << "done with sending file" << endl;
        break;
    }

    /*Receive message from server*/
//                nBytes = recvfrom(clientSocket,buffer,1024,0,NULL, NULL);
//    printf("Received from server: %s\n",buffer);

  }

  input.close();
  close(clientSocket);

  return 0;
}