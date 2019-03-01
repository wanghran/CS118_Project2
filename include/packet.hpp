#ifndef PACKET_HPP
#define PACKET_HPP

#define DATA_SIZE 511
#define TOTAL_PACKET_SIZE 524

#include "tcp_header.hpp"

#include <string>

class packet
{
public:

  tcp_header header;
  char data[DATA_SIZE + 1]{'\0'}; // recv
  char total_data[TOTAL_PACKET_SIZE]{'\0'}; // send
  packet();
  packet(char *send_buffer, int buffer_size, unsigned int seq_num, unsigned int ack_num, unsigned short id, unsigned short flag);
  packet(char *recv_buffer);
  ~packet();

  private:
    char *memcopy_send(char *dest, void *src, size_t stride);
    char *memcopy_recv(void *dest, char *src, size_t stride);
};




#endif