#ifndef PACKET_HPP
#define PACKET_HPP

#define DATA_SIZE 511
#define TOTAL_PACKET_SIZE 524

#include "Header.hpp"

#include <string>

class Packet
{
public:

  Header header;
  char data[DATA_SIZE + 1]{'\0'}; // recv
  char total_data[TOTAL_PACKET_SIZE]{'\0'}; // send
  Packet();
  Packet(char *send_buffer, int buffer_size, unsigned int seq_num, unsigned int ack_num, unsigned short id, unsigned short flag);
  Packet(char *recv_buffer);
  ~Packet();

  private:
    char *memcopy_send(char *dest, void *src, size_t stride);
    char *memcopy_recv(void *dest, char *src, size_t stride);
};




#endif
