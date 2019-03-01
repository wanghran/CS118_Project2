#include <iostream>
#include "tcp_header.hpp"

using namespace std;

int main()
{
  unsigned int server_seq = 4321;
  unsigned int server_ack = 12346;
  unsigned short id = 1;
  unsigned short flag = 6; //sync ack
  tcp_header header(server_seq, server_ack, id, flag);
}