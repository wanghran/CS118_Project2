#include <bitset>
#include "tcp_header.hpp"

void tcp_header::set_seq_num(unsigned int seq_num)
{
  this->seq_num = seq_num;
}

void tcp_header::set_ack_num(unsigned int ack_num)
{
  this->ack_num = ack_num;
}

void tcp_header::set_ID(unsigned short ID)
{
  this->ID = ID;
}

void tcp_header::set_flag(unsigned short flag)
{
  this->flag = flag;
}

tcp_header::tcp_header(unsigned int seq_num, unsigned int ack_num,
                       unsigned short ID, unsigned short flag)
{
  tcp_header::set_seq_num(seq_num);
  tcp_header::set_ack_num(ack_num);
  tcp_header::set_ID(ID);
  tcp_header::set_flag(flag);
}

std::string tcp_header::give_header()
{
  std::bitset<32> seq(seq_num);
  std::bitset<32> ack(ack_num);
  std::bitset<16> id(ID);
  std::bitset<16> flag_(flag);
  return seq.to_string() + ack.to_string() + id.to_string() + flag_.to_string();
}

unsigned int tcp_header::give_seq(std::string header)
{
  std::bitset<32> seq(header.substr(0, 32));
  return (unsigned int)seq.to_ulong();
}

unsigned int tcp_header::give_ack(std::string header)
{
  std::bitset<32> ack(header.substr(32, 64));
  return (unsigned int)ack.to_ulong();
}

unsigned short tcp_header::give_id(std::string header)
{
  std::bitset<16> id(header.substr(64, 80));
  return (unsigned short)id.to_ulong();
}

unsigned short tcp_header::give_flag(std::string header)
{
  std::bitset<16> flag(header.substr(80, 96));
  return (unsigned short)flag.to_ulong();
}