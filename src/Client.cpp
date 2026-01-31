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

void Client::fill_buffer()
{
    std::cout  << "debut fill buffer" << std::endl;
    char buffer[513];
    ssize_t bytes;
    bytes = recv(_socket, buffer, 512, 0);

    while (bytes > 0)
    {
        bytes = recv(_socket, buffer, 512, 0);
        buffer[bytes] = '\0';
        _recv_buff.append(buffer, bytes);
        std::cout  << "recv buff : " << _recv_buff << std::endl;
        parse_buffer();
    }
    if (bytes == 0)
        std::cout << "Client " << _socket << " disconnected" << std::endl;
    else 
    {
        if (errno == EAGAIN && errno == EWOULDBLOCK)
        {
            std::cout << "No more data" << std::endl;
            return ;
        }
        throw std::runtime_error(std::string("recv :") + ::strerror(errno));
    }
}

void Client::parse_buffer()
{
    size_t pos;
    while ((pos = _recv_buff.find("\r\n")) != std::string::npos)
    {
        std::string msg = _recv_buff.substr(0, pos);
        _recv_buff.erase(0, pos + 2);  
        
        if (!msg.empty())
            parse_msg(msg);
    }
}

void Client::parse_msg(std::string msg)
{
    std::cout << msg << std::endl;
}