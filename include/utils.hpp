#ifndef UTILS_HPP
#define UTILS_HPP

#define SERVER_DATA_START_SEQ_NUM 4322
#define CLIENT_DATA_START_SEQ_NUM 12346
#define DATA_BUFFER_SIZE 512
#define MAX_NUM 102400

int carry = 0;

int server_convert_to_packet_id(int byte_num) {
    return int((byte_num + carry * (MAX_NUM + 1) - CLIENT_DATA_START_SEQ_NUM) /
               DATA_BUFFER_SIZE);
}

int client_convert_to_packet_id(int byte_num, int recv_bytes) {
    return server_convert_to_packet_id(byte_num - recv_bytes);
}

int client_get_next_seq_num(int seq_num) {
    int rtn = seq_num + DATA_BUFFER_SIZE;
    if (rtn > MAX_NUM) {
        ++carry;
        return rtn - MAX_NUM - 1;
    } else {
        return rtn;
    }
}



#endif
