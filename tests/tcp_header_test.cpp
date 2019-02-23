#include <iostream>
#include "tcp_header.hpp"

int main()
{
  unsigned int server_seq = 4321;
  unsigned int server_ack = 12346;
  unsigned short id = 1;
  unsigned short flag = 6; //sync ack
  tcp_header header(server_seq, server_ack, id, flag);
  std::cout << (tcp_header::give_seq(header.give_header()) == server_seq) << std::endl;
  std::cout << (tcp_header::give_ack(header.give_header()) == server_ack) << std::endl;
  std::cout << (tcp_header::give_id(header.give_header()) == id) << std::endl;
  std::cout << (tcp_header::give_flag(header.give_header()) == flag) << std::endl;
}