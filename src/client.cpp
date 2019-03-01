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

#include "packet.hpp"

//http://developerweb.net/viewtopic.php?id=3196

using namespace std;
#define buffer_size 512
struct addrinfo hints, *infoptr;

int main(int argc, char* argv[]){

  if(argc != 4){
    cerr << "ERROR: Number of arguments incorrect. Try ./client <HOSTNAME/IP> <PORT> <FILENAME>\n";
    exit(EXIT_FAILURE);
  }

  char* host_name = argv[1];
  int port = atoi(argv[2]);
  char* file_name = argv[3];


  if(port < 1024 || port > 65535){
    cerr << "ERROR: Port numbers incorrect. Try another one.\n";
    exit(EXIT_FAILURE);
  }


  int clientSocket, portNum, nBytes;
  char buffer[buffer_size];
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

  ifstream input (file_name,  ios::binary);
  if(!input.is_open()){
      cerr << "ERROR: cannot open the file" << endl;
      exit(EXIT_FAILURE);
  }



  while(1){
    memset(buffer, '\0', sizeof(buffer));
    // shared_ptr<packet> pack(new packet(buffer, buffer_size, 12345, 4321, 0, 4));
    // std::stringstream ss;
    // ss.read((char*)&pack, sizeof(pack));
    int bytes_send = input.read(buffer, sizeof(buffer)).gcount();

    if (sendto(clientSocket,buffer,bytes_send,0,(struct sockaddr *)&serverAddr,addr_size) < 0) {
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