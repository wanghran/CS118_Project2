#include <cstring>
#include <iostream>
#include "packet.hpp"
#include <cassert>
using namespace std;
// using char_array_ptr = std::unique_ptr<char[]>;
//have to manual add \0 to ransfer string data, however, it does not work for binary data
packet::packet() {}

packet::packet(char *send_buffer, int buffer_size, unsigned int seq_num,
               unsigned int ack_num, unsigned short id, unsigned short flag) : header(seq_num, ack_num, id, flag)
{
  // Encoder.
  cout << "$$ header.seq_num " << header.seq_num << endl;
  cout << sizeof(header.seq_num) << endl;
  char *ptr = total_data;
  ptr = memcopy_send(ptr, (void *)&header.seq_num, sizeof(header.seq_num));
  ptr = memcopy_send(ptr, (void *)&header.ack_num, sizeof(header.ack_num));
  ptr = memcopy_send(ptr, (void *)&header.ID, sizeof(header.ID));
  ptr = memcopy_send(ptr, (void *)&header.flag, sizeof(header.flag));
  ptr = memcopy_send(ptr, send_buffer, buffer_size);
  // assert(total_data[TOTAL_PACKET_SIZE - 1] == '\0');
}

packet::packet(char *recv_buffer)
{
  // Decoder.
  cout << "$$ header.seq_num" << header.seq_num << endl;
  cout << sizeof(header.seq_num) << endl;
  char *ptr = recv_buffer;
  ptr = memcopy_recv((void *)&header.seq_num, ptr, sizeof(header.seq_num));
  ptr = memcopy_recv((void *)&header.ack_num, ptr, sizeof(header.ack_num));
  ptr = memcopy_recv((void *)&header.ID, ptr, sizeof(header.ID));
  ptr = memcopy_recv((void *)&header.flag, ptr, sizeof(header.flag));
  ptr = memcopy_recv(data, ptr, DATA_SIZE);
  assert(data[DATA_SIZE] == '\0');
}

packet::~packet() {}

char *packet::memcopy_send(char *dest, void *src, size_t stride) {
  memcpy(dest, src, stride);
  dest += stride;
  return dest;
}

char *packet::memcopy_recv(void *dest, char *src, size_t stride)
{
  memcpy(dest, src, stride);
  src += stride;
  return src;
}
