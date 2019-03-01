#include <iostream>
#include "packet.hpp"
#include <cstring>
#include <memory>
#include <fstream>
#include <sstream>

using namespace std;
int main()
{
  fstream input("/vagrant/tests/files/input.txt", ios::in | ios::binary);
  int size = 0;
  if (input.is_open()) {
    input.seekg(0, input.end);
    size = input.tellg();
    input.seekg(0, input.beg);
  }
  char buffer[size];
  input.read(buffer, size);
  cout << "@@@ " << buffer << endl;
  // shared_ptr<packet> pack(new packet(buffer, size, 12345, 4321, 0, 4));
  packet send_pack(buffer, size, 12345, 4321, 0, 4);
  cout << "pack.data (don't look at this): " << send_pack.total_data << endl;
  packet recv_pack(send_pack.total_data);
  cout << "recv_pack.header.seq_num " << recv_pack.header.seq_num << endl;
  cout << "recv_pack.header.ack_num " << recv_pack.header.ack_num << endl;
  cout << "recv_pack.header.ID " << recv_pack.header.ID << endl;
  cout << "recv_pack.header.flag " << recv_pack.header.flag << endl;
  cout << "recv_pack.data " << recv_pack.data << endl;

  // stringstream ss;
  // ss.read((char*)&pack, sizeof(pack));
  // char obj[sizeof(pack)]{'\0'};
  // ss.write(obj, sizeof(obj));
  // cout << "after write" << endl;
  // cout << "@@@@ " << obj << endl;
  // cout << sizeof(obj) << endl;
  // stringstream ss2;
  // ss2.read(obj, sizeof(obj));
  // packet pack2;
  // ss2.write((char*)&pack2, sizeof(pack2));
  // cout << pack2.yyy << endl;
  // cout << pack2.data2 << endl;
  // cout << pack2.data << endl;
  // std::cout << "buffer " << buffer << " end" << std::endl;
  // std::cout << "buffer size " << size << " end" << std::endl;
  // std::cout << "@@@" << TEMP << "###" << std::endl;
  return 0;
}