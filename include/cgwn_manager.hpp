#ifndef CGWN_MANAGER_HPP
#define CGWN_MANAGER_HPP

#include <deque>
#include "packet.hpp"

class cgwn_manager
{
  public:
    cgwn_manager(std::deque<packet> &cgwn);
    ~cgwn_manager();

  private:
    std::deque<packet> &cgwn;
};
#endif