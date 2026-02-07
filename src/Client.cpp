#include "Client.hpp"

Client::Client(int socket, sockaddr_in addr): _socket(socket), _addr(addr), _status(0)
{
    (void)_socket;
    (void)_addr;
}

Client::~Client()
{
    close(_socket);
}

void Client::fill_recv_buffer()
{
    std::cout  << "debut fill buffer" << std::endl;
    char buffer[513];
    ssize_t bytes;

    while ((bytes = recv(_socket, buffer, 512, 0)) > 0)
    {
        buffer[bytes] = '\0';
        _recv_buff += std::string(buffer, bytes);
        std::cout  << "recv buff : " << _recv_buff << std::endl;
        parse_buffer();
    }
    if (bytes == 0)
        std::cout << "Client " << _socket << " disconnected" << std::endl;
    else 
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            std::cout << "No more data" << std::endl;
            return ;
        }
        throw std::runtime_error(std::string("recv :") + ::strerror(errno));
    }
}
