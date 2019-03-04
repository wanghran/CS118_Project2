#include <cassert>
#include <netinet/in.h>
#include <iostream>
#include <cstdint>
#include "tcp_header.hpp"

using namespace std;

tcp_header::tcp_header() {}

void tcp_header::set_seq_num(uint32_t seq_num)
{
  this->seq_num = htonl(seq_num);
}

void tcp_header::set_ack_num(uint32_t ack_num)
{
  this->ack_num = htonl(ack_num);
}

void tcp_header::set_ID(uint16_t ID)
{
  this->ID = htons(ID);
}

void tcp_header::set_flag(uint16_t flag)
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

uint32_t tcp_header::give_seq(tcp_header &recv_header)
{
  return ntohl(recv_header.seq_num);
}

uint32_t tcp_header::give_ack(tcp_header &recv_header)
{
  return ntohl(recv_header.ack_num);
}

uint16_t tcp_header::give_id(tcp_header &recv_header)
{
  return ntohs(recv_header.ID);
}

uint16_t tcp_header::give_flag(tcp_header &recv_header)
{
  return ntohs(recv_header.flag);
}