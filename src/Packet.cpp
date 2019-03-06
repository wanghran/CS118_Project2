#include <cstring>
#include <iostream>
#include "Packet.hpp"
#include <cassert>
using namespace std;
// using char_array_ptr = std::unique_ptr<char[]>;
//have to manual add \0 to ransfer string data, however, it does not work for binary data
Packet::Packet() {}

Packet::Packet(char *send_buffer, int buffer_size, unsigned int seq_num,
               unsigned int ack_num, unsigned short id, unsigned short flag) : header(seq_num, ack_num, id, flag)
{
    // Encoder.
    char *ptr = total_data;
    data_bytes = buffer_size;
    ptr = memcopy_send(ptr, (void *)&header.seq_num, sizeof(header.seq_num));
    ptr = memcopy_send(ptr, (void *)&header.ack_num, sizeof(header.ack_num));
    ptr = memcopy_send(ptr, (void *)&header.ID, sizeof(header.ID));
    ptr = memcopy_send(ptr, (void *)&header.flag, sizeof(header.flag));
    ptr = memcopy_send(ptr, send_buffer, buffer_size);
    // assert(total_data[TOTAL_PACKET_SIZE - 1] == '\0');
}

Packet::Packet(char *recv_buffer, int bytes_recved)
{
    // Decoder.
    char *ptr = recv_buffer;
    ptr = memcopy_recv((void *)&header.seq_num, ptr, sizeof(header.seq_num));
    ptr = memcopy_recv((void *)&header.ack_num, ptr, sizeof(header.ack_num));
    ptr = memcopy_recv((void *)&header.ID, ptr, sizeof(header.ID));
    ptr = memcopy_recv((void *)&header.flag, ptr, sizeof(header.flag));
    int num_data_bytes = bytes_recved - HEADER_SIZE;
    ptr = memcopy_recv(data, ptr, num_data_bytes);
    data_bytes = num_data_bytes;
    assert(data[num_data_bytes] == '\0');
}

Packet::~Packet() {}

void Packet::send_packet(const Conn &conn) {
    if (state != INIT) {
        return; // do not send it again if it is already sent but not acked yet unless timeout
    }
    if (sendto(conn.clientSocket, total_data,
               data_bytes + HEADER_SIZE, 0,
               (struct sockaddr *)&conn.addr, conn.addr_size) < 0) {
        perror("send to");
        exit(EXIT_FAILURE);
    } else {
        state = SENT;
    }
}

char *Packet::memcopy_send(char *dest, void *src, size_t stride) {
    memcpy(dest, src, stride);
    dest += stride;
    return dest;
}

char *Packet::memcopy_recv(void *dest, char *src, size_t stride)
{
    memcpy(dest, src, stride);
    src += stride;
    return src;
}
