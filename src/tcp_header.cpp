#include <bitset>
#include "tcp_header.hpp"
#include <cassert>
#include <netinet/in.h>
#include <iostream>
// #include <stdio.h>
// #include <stdlib.h>

using namespace std;

tcp_header::tcp_header() {}


void tcp_header::set_seq_num(unsigned int seq_num)
{
  this->seq_num = htonl(seq_num);
}

void tcp_header::set_ack_num(unsigned int ack_num)
{
  this->ack_num = htonl(ack_num);
}

void tcp_header::set_ID(unsigned short ID)
{
  this->ID = htons(ID);
}

void tcp_header::set_flag(unsigned short flag)
{
  this->flag = htons(flag);
}

tcp_header::tcp_header(unsigned int seq_num, unsigned int ack_num,
                       unsigned short ID, unsigned short flag)
{
  tcp_header::set_seq_num(seq_num);
  tcp_header::set_ack_num(ack_num);
  tcp_header::set_ID(ID);
  tcp_header::set_flag(flag);
}

// unsigned int tcp_header::give_seq(std::string header)
// {
//   std::bitset<32> seq(header.substr(0, 32));
//   return (unsigned int)seq.to_ulong();
// }

// unsigned int tcp_header::give_ack(std::string header)
// {
//   std::bitset<32> ack(header.substr(32, 64));
//   return (unsigned int)ack.to_ulong();
// }

// unsigned short tcp_header::give_id(std::string header)
// {
//   std::bitset<16> id(header.substr(64, 80));
//   return (unsigned short)id.to_ulong();
// }

// unsigned short tcp_header::give_flag(std::string header)
// {
//   std::bitset<16> flag(header.substr(80, 96));
//   return (unsigned short)flag.to_ulong();
// }