#include <cstring>
#include <iostream>
#include "packet.hpp"

// using char_array_ptr = std::unique_ptr<char[]>;

packet::packet(char *buffer, unsigned int seq_num, unsigned int ack_num, unsigned short id, unsigned short flag)
{
  content = buffer;
  tcp_header temp(seq_num, ack_num, id, flag);
  header = temp;
  const char *head = header.give_header().c_str();
  int header_size = header.give_header().length();
  int total_size = header_size + sizeof(content);
  data = new char[total_size]{'\0'};
  std::memcpy(data, head, header_size);
  std::memcpy(data + header_size, content, sizeof(content));
}

packet::~packet()
{
  delete data;
  data = 0;
}
