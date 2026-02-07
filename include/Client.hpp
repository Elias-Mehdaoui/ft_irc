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

        int _status; // 0 = didnt give password // 1 = password ok no NICK no USER // 2 = password + NICK ok no USER // 2 registered
    
    public:
        Client(int socket, sockaddr_in addr);
        ~Client();
        void fill_recv_buffer();
        void fill_send_buffer(std::string msg);
        void flush_send();
        void parse_msg(std::string msg);
        bool is_registered();
        void clear_recv_buff();
        
        std::string get_recv_buff();
        int get_status();

        void set_password(std::string password);
        void set_nick(std::string nick);
        void set_user(std::string user);
        void set_recv_buff(std::string buff);
        void set_status(int status);


};



#endif