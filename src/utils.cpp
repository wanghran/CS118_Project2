#include "utils.hpp"


//int send_carry = 0;
int carry = 0;
int cur_max_byte_num = 0;

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
//    cout << "###@@ byte_num " << byte_num << " " << " recv_bytes " << recv_bytes << " " << " carry " << carry << " cur_max_byte_num " << cur_max_byte_num << " rtn " << server_convert_to_packet_id(byte_num - recv_bytes) - 1 << endl;
    if (byte_num < cur_max_byte_num / 20) { // too hacky: server tells client that the carry happened
        cur_max_byte_num = 0;
        ++carry;
    } else {
        if (byte_num > cur_max_byte_num) {
            cur_max_byte_num = byte_num;
        }
    }
    return server_convert_to_packet_id(byte_num - recv_bytes) - 1; // 1-based to 0-based
} // ###@@ 12858 524 0
// ###@@ 12858 12 1

int client_get_next_seq_num(int seq_num)
{
    int rtn = seq_num;
    if (rtn > MAX_NUM)
    {
//        cout << "    @@@ " << carry << " " << rtn << " " << MAX_NUM << endl;
//        ++send_carry;
        return rtn - MAX_NUM - 1;
    }
    else
    {
//        cout << " ###" << rtn << endl;
        return rtn;
    }
}
