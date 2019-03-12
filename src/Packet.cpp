#include "Packet.hpp"
#include "utils.hpp"

#include <cstring>
#include <iostream>
#include <cassert>
#include <unistd.h>
#include <errno.h>

using std::cout;
using std::endl;

#define TOTAL_BUFFER_SIZE 524

timestamp most_recent_time_no_recved;
bool no_recv_check_exit_can_check = false;

void no_recv_check_exit();

Packet::Packet() {}

Packet::Packet(char *send_buffer, int buffer_size, unsigned int seq_num,
               unsigned int ack_num, unsigned short id, unsigned short flag) : header(seq_num, ack_num, id, flag)
{
    // Encoder.
    char *ptr = total_data;
    data_bytes = buffer_size;
    total_bytes = data_bytes + HEADER_SIZE;
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
    total_bytes = bytes_recved;
    data_bytes = bytes_recved - HEADER_SIZE;
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
    if (sendto(conn.socket, total_data,
               data_bytes + HEADER_SIZE + 1, 0,
               (struct sockaddr *)&conn.addr, conn.addr_size) < 0) {
        perror("send to");
        exit(EXIT_FAILURE);
    } else {
        state = SENT;
        send_time = high_resolution_clock::now();
    }
}

void Packet::print_packet() const {
    D(cout << "----------\n";)
    D(cout << "header.seq_num " << ntohl(header.seq_num) << endl;)
    D(cout << "header.ack_num " << ntohl(header.ack_num) << endl;)
    D(cout << "header.ID " << ntohs(header.ID) << endl;)
    D(cout << "header.flag " << ntohs(header.flag) << endl;)
    D(cout << "data " << data << endl;)
    D(cout << "----------\n\n";)
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

bool Packet::is_timeout() const {
    if (state == SENT) {
        timestamp current_time = high_resolution_clock::now();
        double duration = std::chrono::duration_cast
        <std::chrono::milliseconds>(current_time - send_time).count();
//        cout << duration << endl;
        return duration >= 500; // 0.5 sec
    }
    return false;
}


void Packet::official_recv_print(bool is_client, int cwnd, int ss_thresh) {
    cout << "RECV " << Header::give_seq(header) << " " << Header::give_ack(header) << " " << Header::give_id(header);
    if (is_client) {
        cout << " " << cwnd << " " << ss_thresh;
    }
    cout << " ";
    switch (Header::give_flag(header)) {
        case SYN_ACK: {
            cout << "ACK SYN";
            break;
        }
        case FIN_ACK: {
            cout << "ACK FIN";
            break;
        }
        case ACK: {
            cout << "ACK";
            break;
        }
        case SYN: {
            cout << "SYN";
            break;
        }
        case FIN: {
            cout << "FIN";
            break;
        }
    }
    cout << endl;
}


void Packet::official_send_print(bool is_client, int cwnd, int ss_thresh, bool is_dup) {
    cout << "SEND " << Header::give_seq(header) << " " << Header::give_ack(header) << " " << Header::give_id(header);
    if (is_client) {
        cout << " " << cwnd << " " << ss_thresh;
    }
    cout << " ";
    if (is_dup) {
        cout << "DUP";
    }
    else {
        switch (Header::give_flag(header)) {
            case SYN_ACK: {
                cout << "ACK SYN";
                break;
            }
            case FIN_ACK: {
                cout << "ACK FIN";
                break;
            }
            case ACK: {
                cout << "ACK";
                break;
            }
            case SYN: {
                cout << "SYN";
                break;
            }
            case FIN: {
                cout << "FIN";
                break;
            }
        }
    }
    cout << endl;
}

void Packet::official_drop_print()
{
    cout << "DROP " << Header::give_seq(header) << " " << Header::give_ack(header) << " " << Header::give_id(header) << " ";
    switch (Header::give_flag(header))
        {
        case SYN_ACK:
        {
            cout << "ACK SYN";
            break;
        }
        case FIN_ACK:
        {
            cout << "ACK FIN";
            break;
        }
        case ACK:
        {
            cout << "ACK";
            break;
        }
        case SYN:
        {
            cout << "SYN";
            break;
        }
        case FIN:
        {
            cout << "FIN";
            break;
        }
    }
    cout << endl;
}

shared_ptr<Packet> recv_packet(Conn &conn) {
    char buffer[TOTAL_BUFFER_SIZE];
    //    FD_ZERO(&conn.read_fds);
    memset(buffer, '\0', sizeof(buffer));
    struct timeval timeout;
    timeout.tv_sec = 0; // TODO: fix server
    timeout.tv_usec = 500000; // 0.5 sec TODO: check!!!!!!!
            D(cout << "xxx if select" << endl;)
    if (select(conn.socket + 1, &conn.read_fds, NULL, NULL, &timeout) > 0) {
                D(cout << "@@@yes" << endl;)
        no_recv_check_exit_can_check = false;
        int n_bytes = int(recvfrom(conn.socket, buffer, sizeof(buffer), 0,
                                   (struct sockaddr *)&conn.addr, &conn.addr_size));
        //        printf("%s\n", strerror(errno));
        D(cout << "n_bytes " << n_bytes << endl;)
        return shared_ptr<Packet>(new Packet(buffer, n_bytes));
    } else {
                D(cout << "@@@no" << endl;)
        // TODO: potentially reset...
        // Whenever client receives no packets from server for more than 10 seconds , it
        // should abort the connection (close socket and exit with non-zero code).
        no_recv_check_exit();
        return nullptr;
    }
}

void no_recv_check_exit() {
    if (no_recv_check_exit_can_check) {
        timestamp cur = high_resolution_clock::now();
        double duration = std::chrono::duration_cast
        <std::chrono::milliseconds>(cur - most_recent_time_no_recved).count();
        if (duration > 10000) {
                    cout << "ABORT: finally " << duration << " msecs passed" << endl;
            exit(EXIT_FAILURE);
        }
//        cout << "@" << endl;
    } else {
        most_recent_time_no_recved = high_resolution_clock::now();
        no_recv_check_exit_can_check = true;
//        cout << "@@@last_time_something_recved " << endl;
    }
}

