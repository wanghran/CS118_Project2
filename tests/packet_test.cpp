#include <iostream>
#include "packet.hpp"
#include <cstring>
#include <memory>

using namespace std;
int main()
{
  char* content = "xxxxxx";
  shared_ptr<packet> pack(new packet(content, 12345, 4321, 0, 4));
  char* TEMP = pack->data;
  std::cout << "@@@" << TEMP << "###" << std::endl;
  return 0;
}