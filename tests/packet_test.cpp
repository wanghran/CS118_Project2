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
  packet send_pack(buffer, size, 12345, 4321, 0, 4);
  cout << "pack.data (don't look at this): " << send_pack.total_data << endl;
  packet recv_pack(send_pack.total_data);
  cout << "recv_pack.header.seq_num " << recv_pack.header.seq_num << endl;
  cout << "recv_pack.header.ack_num " << recv_pack.header.ack_num << endl;
  cout << "recv_pack.header.ID " << recv_pack.header.ID << endl;
  cout << "recv_pack.header.flag " << recv_pack.header.flag << endl;
  cout << "recv_pack.data " << recv_pack.data << endl;

  return 0;
}