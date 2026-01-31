#ifndef SERVER_HPP_
#define SERVER_HPP_


#include "Client.hpp"

class Server 
{
    private :
        int _port;
        std::string _password;
        int _serverSocket;
        sockaddr_in _serverAddress;
        std::map<int, Client *> _clients;
        bool _is_running;

    
    public :
        Server(std::string port, std::string password);
        ~Server();

        void run();

        

};


#endif