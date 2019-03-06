#ifndef CONN_HPP
#define CONN_HPP

#include <sys/socket.h>
#include <netinet/in.h>


class Conn {
public:
    int clientSocket;
    sockaddr_in addr;
    socklen_t addr_size;
};

#endif
