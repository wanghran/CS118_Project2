#ifndef PACKET_HPP
#define PACKET_HPP

#include "tcp_header.hpp"

class packet
{
private:
    tcp_header header;
    char* content;

public:
  char *data; // new total_size of bytes; will be deallocated by dtor
  packet(char *buffer, unsigned int seq_num, unsigned int ack_num, unsigned short id, unsigned short flag);
  ~packet();
  char* give_packet(); 


};


#endif