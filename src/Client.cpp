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
            return ;
        throw std::runtime_error(std::string("recv :") + ::strerror(errno));
    }
}

void Client::fill_send_buffer(std::string msg)
{
    _send_buff += msg;
}

void Client::flush_send()
{
    std::cout << "Sending :" << _send_buff << std::endl;
    send(_socket, _send_buff.c_str(), _send_buff.size(), 0);
    _send_buff.erase();
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

void Client::set_password(std::string password)
{
    _password = password;
}

void Client::set_nick(std::string nick)
{
    _nickname = nick;
}

void Client::set_user(std::string user)
{
    _username = user;
}

std::string Client::get_nickname()
{
    return _nickname;
}

std::string Client::get_username()
{
    return _username;
}

bool Client::is_registered()
{
    return _status == 3;
}

void Client::clear_recv_buff()
{
    _recv_buff.clear();
}

