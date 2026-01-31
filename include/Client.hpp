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
#include <errno.h>

class Client
{
    private :
        int _socket;
        sockaddr_in _addr;
        std::string _nickname;
        std::string _username;
    
    public:
        Client(int socket, sockaddr_in addr);
        ~Client();

};



#endif