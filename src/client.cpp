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
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <vector>
#include <memory>

#include "Packet.hpp"

//http://developerweb.net/viewtopic.php?id=3196

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


struct addrinfo hints, *infoptr;

void printInt_32(uint32_t x);
void printInt_16(uint16_t x);
void syn(int clientSocket, sockaddr_in serverAddr, socklen_t addr_size);
void fin(int clientSocket, sockaddr_in serverAddr, socklen_t addr_size);

class ClientData {
public:
    vector<shared_ptr<Packet>> packets;
    int left = 0;
    int right = 0;
};



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
 //  puts(host);
 //  (above) check hostname and change hostname to ip



  int clientSocket, portNum, nBytes;
  char buffer[DATA_BUFFER_SIZE - 1]; //the 512th byte in the buffer is set to be \0, only read 511
  struct sockaddr_in serverAddr;
  socklen_t addr_size;

  /*Create UDP socket*/
  clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (clientSocket< 0) {
        fprintf(stderr, "ERROR: socket creation failed\n");
        exit(1);
    }

  /*Configure settings in address struct*/
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

  /*Initialize size variable to be used later on*/
  addr_size = sizeof serverAddr;


    cout << "Client " << port << endl;
  syn(clientSocket,serverAddr,addr_size);



// transmission actual data
  ifstream input (file_name, ios::binary);
  if(!input.is_open()){
      cerr << "ERROR: cannot open the file" << endl;
      exit(EXIT_FAILURE);
  }



  while(1){
    memset(buffer, '\0', sizeof(buffer));
    int bytes_send = input.read(buffer, sizeof(buffer)).gcount();
    cout << "byte send: " << bytes_send << endl;
    Packet pack(buffer, DATA_BUFFER_SIZE, 12345, 4321, 1, 4);


    if (sendto(clientSocket,pack.total_data,bytes_send+12,0,(struct sockaddr *)&serverAddr,addr_size) < 0) {
        perror("send to");
        exit(EXIT_FAILURE);
    }


    if (bytes_send == 0){
        cout << "done with sending file" << endl;
        break;
    }

    /*Receive ack from server*/

    // regular data ack;
//    char ACK_buffer[TOTAL_BUFFER_SIZE];
//    int nBytes_ACK = recvfrom(clientSocket,ACK_buffer,sizeof(ACK_buffer),0,(struct sockaddr *)&serverAddr,&addr_size);
//    packet ACK_pack(ACK_buffer);



  }

  input.close();
//  fin(clientSocket,serverAddr,addr_size);
  close(clientSocket);
  return 0;
}



void printInt_32(uint32_t x)
{
    cout << setfill('0') << setw(8) << hex << x << '\n';
}

void printInt_16(uint16_t x)
{
    cout << setfill('0') << setw(4) << hex << x << '\n';
}


void syn(int clientSocket, sockaddr_in serverAddr, socklen_t addr_size) {
    
    char SYN_buffer[DATA_BUFFER_SIZE - 1];
    memset(SYN_buffer, '\0', sizeof(SYN_buffer));
    Packet SYN_pack(SYN_buffer, DATA_BUFFER_SIZE, 12345, 0, 0, SYN);
    sendto(clientSocket,SYN_pack.total_data,TOTAL_BUFFER_SIZE,0,(struct sockaddr *)&serverAddr,addr_size);
    //    printInt_16(SYN_pack.header.flag);
    
    while(1){
        char SYN_ACK_buffer[TOTAL_BUFFER_SIZE];
        int nBytes_SYN_ACK = recvfrom(clientSocket,SYN_ACK_buffer,sizeof(SYN_ACK_buffer),0,(struct sockaddr *)&serverAddr,&addr_size);
        Packet SYN_ACK_pack(SYN_ACK_buffer);
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

//needs to change
void fin(int clientSocket, sockaddr_in serverAddr, socklen_t addr_size) {
    
    char SYN_buffer[DATA_BUFFER_SIZE - 1];
    memset(SYN_buffer, '\0', sizeof(SYN_buffer));
    Packet SYN_pack(SYN_buffer, DATA_BUFFER_SIZE, 12345, 0, 0, SYN);
    sendto(clientSocket,SYN_pack.total_data,TOTAL_BUFFER_SIZE,0,(struct sockaddr *)&serverAddr,addr_size);
    //    printInt_16(SYN_pack.header.flag);
    
    while(1){
        char SYN_ACK_buffer[TOTAL_BUFFER_SIZE];
        int nBytes_SYN_ACK = recvfrom(clientSocket,SYN_ACK_buffer,sizeof(SYN_ACK_buffer),0,(struct sockaddr *)&serverAddr,&addr_size);
        Packet SYN_ACK_pack(SYN_ACK_buffer);
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







