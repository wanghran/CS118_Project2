#ifndef PACKET_HPP
#define PACKET_HPP

#define DATA_SIZE 511
#define HEADER_SIZE 12
#define TOTAL_PACKET_SIZE 524

#include "Header.hpp"
#include "Conn.hpp"

#include <string>
#include <chrono>
#include <memory>

typedef std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;

using std::chrono::high_resolution_clock;

using std::shared_ptr;


enum State {INIT, SENT, ACKED};

class Packet
{
public:
    Header header;
    char data[DATA_SIZE + 1]{'\0'}; // recv
    char total_data[TOTAL_PACKET_SIZE]{'\0'}; // send
    int data_bytes = 0;
    int total_bytes = 0;
    State state = INIT;
    timestamp send_time;
    Packet();
    Packet(char *send_buffer, int buffer_size, unsigned int seq_num, unsigned int ack_num, unsigned short id, unsigned short flag);
    Packet(char *recv_buffer, int bytes_recved);
    void send_packet(const Conn &conn);
    void print_packet() const;
    ~Packet();
    
private:
    char *memcopy_send(char *dest, void *src, size_t stride);
    char *memcopy_recv(void *dest, char *src, size_t stride);
};

shared_ptr<Packet> recv_packet(Conn &conn);


#endif
