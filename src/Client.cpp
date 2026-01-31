#include "Client.hpp"

Client::Client(int socket, sockaddr_in addr): _socket(socket), _addr(addr)
{
    (void)_socket;
    (void)_addr;
}

Client::~Client()
{}
