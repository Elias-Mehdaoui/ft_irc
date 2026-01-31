#ifndef SERVER_HPP
#define SERVER_HPP


#include "Client.hpp"

#define EVENTS_MAX 10

class Server 
{
    private :
        int _port;
        std::string _password;
        int _serverSocket;
        sockaddr_in _serverAddress;
        std::map<int, Client *> _clients;
        bool _is_running;
        int _epollfd;

    
    public :
        Server(std::string port, std::string password);
        ~Server();

        void run();
        void handle_event(struct epoll_event event);
        void new_client();
        

};


#endif