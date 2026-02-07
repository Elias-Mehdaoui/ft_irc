#ifndef SERVER_HPP
#define SERVER_HPP


#include "Client.hpp"

# define EVENTS_MAX 10
# define SERVER_NAME ":irc.serv.42" 

# define RPL_WELCOME(nickName) ((std::string)SERVER_NAME + "001 " + nickName + " :Welcome to my irc server, " + nickName + "\r\n");
# define RPL_YOURID(nickName) ((std::string)SERVER_NAME + "002 " + nickName + " :" + "Your host is " + SERVER_NAME + ", running version irc-1.0" + "\r\n");
# define RPL_YOURHOST(nickName) ((std::string)SERVER_NAME + "003 " + nickName + " :" + "This server was created Tue Mars 23 2024 at 22:15:05 CEST" + "\r\n");
# define RPL_MYINFO(nickName) ((std::string)SERVER_NAME + "004 " + nickName + " " + SERVER_NAME + "\r\n");


class Server 
{
    private :
        int _port;
        std::string _password;
        int _serverSocket;
        sockaddr_in _serverAddress;
        std::map<int, Client *> _clients;
        std::map<std::string, void (Server::*)(Client *, std::string)> _commands;
        bool _is_running;
        int _epollfd;

    
    public :
        Server(std::string port, std::string password);
        ~Server();

        void run();
        void handle_event(struct epoll_event event);
        void new_client();
        void fill_buffer(int client_socket);
        void parse_buffer(int client_socket);
        void parse_msg(std::string msg, int client_socket);

        // Commands
        void CAP(Client *client, std::string cmd);
        void PASS(Client *client, std::string cmd);
        void NICK(Client *client, std::string cmd);
        void USER(Client *client, std::string cmd);
};


#endif