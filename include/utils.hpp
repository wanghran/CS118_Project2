#ifndef UTILS_HPP
#define UTILS_HPP

//#define DEBUG


#ifdef DEBUG
#define D(x) x
#else
#define D(x)
#endif



#define SERVER_DATA_START_SEQ_NUM 4322

#include <math.h>
#include <iostream>


#define CLIENT_DATA_START_SEQ_NUM 12346
#define DATA_BUFFER_SIZE 512
#define MAX_NUM 102400

int server_convert_to_packet_id(int byte_num);
int server_convert_to_byte_num(int packet_id);
// with carry
int server_convert_to_packet_id_with_carry(int byte_num, int c);
int server_convert_to_byte_num_with_carry(int packet_id, int c);
int client_convert_to_packet_id(int byte_num, int recv_bytes);
int client_get_next_seq_num(int seq_num);
#endif
