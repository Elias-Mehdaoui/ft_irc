#include "Client.hpp"

Client::Client(int socket, sockaddr_in addr): _socket(socket), _addr(addr), _status(0)
{
    (void)_socket;
    (void)_addr;
    _recv_buff = "";
}

Client::~Client()
{
    close(_socket);
}

void Client::fill_recv_buffer()
{
    char buffer[513];
    ssize_t bytes;

    while ((bytes = recv(_socket, buffer, 512, 0)) > 0)
    {
        buffer[bytes] = '\0';
        _recv_buff += std::string(buffer, bytes);
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

void Client::fill_send_buffer(std::string msg)
{
    _send_buff += msg;
}

void Client::flush_send()
{
    std::cout << "Sending : " << _send_buff << std::endl;
    send(_socket, _send_buff.c_str(), _send_buff.size(), 0);
}

std::string Client::get_recv_buff()
{
    return _recv_buff;
}

int Client::get_status()
{
    return _status;
}


void Client::set_recv_buff(std::string buff)
{
    _recv_buff = buff;
}


void Client::set_status(int status)
{
    _status = status;
}


void Client::clear_recv_buff()
{
    _recv_buff.clear();
}

