#include "utils.hpp"


int carry = 0;

using namespace std;

int server_convert_to_packet_id(int byte_num)
{
    return int(ceil((float)((byte_num + carry * (MAX_NUM + 1) - CLIENT_DATA_START_SEQ_NUM)) /
                    DATA_BUFFER_SIZE));
}
int server_convert_to_byte_num(int packet_id)
{
    return packet_id * DATA_BUFFER_SIZE + CLIENT_DATA_START_SEQ_NUM - carry * (MAX_NUM + 1);
}

// with carry
int server_convert_to_packet_id_with_carry(int byte_num, int c)
{
    return int(ceil((float)((byte_num + c * (MAX_NUM + 1) - CLIENT_DATA_START_SEQ_NUM)) /
                    DATA_BUFFER_SIZE));
}
int server_convert_to_byte_num_with_carry(int packet_id, int c)
{
    return packet_id * DATA_BUFFER_SIZE + CLIENT_DATA_START_SEQ_NUM - c * (MAX_NUM + 1);
}

int client_convert_to_packet_id(int byte_num, int recv_bytes)
{
    cout << "###@@ " << byte_num << " " << recv_bytes << " " << server_convert_to_packet_id(byte_num - recv_bytes) << endl;
    return server_convert_to_packet_id(byte_num - recv_bytes) - 1; // 1-based to 0-based
} // ###@@ 12858 524 0
// ###@@ 12858 12 1

int client_get_next_seq_num(int seq_num)
{
    int rtn = seq_num + DATA_BUFFER_SIZE;
    if (rtn > MAX_NUM)
    {
        ++carry;
        return rtn - MAX_NUM - 1;
    }
    else
    {
        return rtn;
    }
}
