#include <cassert>
#include <netinet/in.h>
#include <iostream>
#include <cstdint>
#include "Header.hpp"

using namespace std;

Header::Header() {}

void Header::set_seq_num(uint32_t seq_num)
{
  this->seq_num = htonl(seq_num);
}

void Header::set_ack_num(uint32_t ack_num)
{
  this->ack_num = htonl(ack_num);
}

void Header::set_ID(uint16_t ID)
{
  this->ID = htons(ID);
}

void Header::set_flag(uint16_t flag)
{
  this->flag = htons(flag);
}

Header::Header(unsigned int seq_num, unsigned int ack_num,
                       unsigned short ID, unsigned short flag)
{
  Header::set_seq_num(seq_num);
  Header::set_ack_num(ack_num);
  Header::set_ID(ID);
  Header::set_flag(flag);
}

uint32_t Header::give_seq(Header &recv_header)
{
  return ntohl(recv_header.seq_num);
}

uint32_t Header::give_ack(Header &recv_header)
{
  return ntohl(recv_header.ack_num);
}

uint16_t Header::give_id(Header &recv_header)
{
  return ntohs(recv_header.ID);
}

uint16_t Header::give_flag(Header &recv_header)
{
  return ntohs(recv_header.flag);
}
