#include "Client.hpp"

Client::Client(int socket, sockaddr_in addr): _socket(socket), _addr(addr)
{}

Client::~Client()
{}
