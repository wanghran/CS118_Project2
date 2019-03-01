#ifndef TCP_HEADER_HPP
#define TCP_HEADER_HPP

#include <string>
#include <exception>


#include <string>

#define HEADER_SIZE 12

class tcp_header
{
  private:
    void set_seq_num(unsigned int seq_num);
    void set_ack_num(unsigned int ack_num);
    void set_ID(unsigned short ID);
    void set_flag(unsigned short flag);

  public:
    unsigned int seq_num = 0;
    unsigned int ack_num = 0;
    unsigned short ID = 0;
    unsigned short flag = 0;
    tcp_header();
    tcp_header(unsigned int seq_num, unsigned int ack_num,
               unsigned short ID, unsigned short flag);
    // static unsigned int give_seq(std::string header);
    // static unsigned int give_ack(std::string header);
    // static unsigned short give_id(std::string header);
    // static unsigned short give_flag(std::string header);
};
#endif