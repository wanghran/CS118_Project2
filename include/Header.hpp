#ifndef HEADER_HPP
#define HEADER_HPP

#include <string>
#include <cstdint>

#define HEADER_SIZE 12

class Header
{
  private:
    void set_seq_num(uint32_t seq_num);
    void set_ack_num(uint32_t ack_num);
    void set_ID(uint16_t ID);
    void set_flag(uint16_t flag);

  public:
    uint32_t seq_num = 0;
    uint32_t ack_num = 0;
    uint16_t ID = 0;
    uint16_t flag = 0;
    Header();
    Header(uint32_t seq_num, uint32_t ack_num,
               uint16_t ID, uint16_t flag);
    static uint32_t give_seq(Header &recv_header);
    static uint32_t give_ack(Header &recv_header);
    static uint16_t give_id(Header &recv_header);
    static uint16_t give_flag(Header &recv_header);
};
#endif
