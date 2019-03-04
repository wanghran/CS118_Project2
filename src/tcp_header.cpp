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

uint32_t tcp_header::give_seq(uint32_t network_seq_num)
{
  return ntohl(network_seq_num);
}

uint32_t tcp_header::give_ack(uint32_t network_ack_num)
{
  return ntohl(network_ack_num);
}

uint16_t tcp_header::give_id(uint16_t network_ID)
{
  return ntohs(network_ID);
}

uint16_t tcp_header::give_flag(uint16_t network_flag)
{
  return ntohs(network_flag);
}