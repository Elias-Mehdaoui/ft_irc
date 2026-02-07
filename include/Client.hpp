#ifndef Client_HPP_
#define Client_HPP_

#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <vector>
#include <map>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <errno.h>
#include <sys/epoll.h>

class Client
{
    private :
        int _socket;
        sockaddr_in _addr;
        std::string _nickname;
        std::string _username;
        std::string _password;
        std::string _recv_buff;
        std::string _send_buff;

        int _status; // 0 = didnt give password // 1 = password ok no NICK or USER // 2 registered
    
    public:
        Client(int socket, sockaddr_in addr);
        ~Client();
        void fill_recv_buffer();
        void fill_send_buffer();
        void parse_buffer();
        void parse_msg(std::string msg);
        bool is_registered();
        void set_status(int status);


};



#endif