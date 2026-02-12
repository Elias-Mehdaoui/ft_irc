#ifndef SERVER_HPP
#define SERVER_HPP


#include "Client.hpp"

# define EVENTS_MAX 10
# define SERVER_NAME ":irc.serv.42" 

# define RPL_WELCOME(nickName) ((std::string)SERVER_NAME + " 001 " + nickName + " :Welcome to my irc server, " + nickName + "\r\n")
# define RPL_YOURID(nickName) ((std::string)SERVER_NAME + " 002 " + nickName + " :" + "Your host is " + SERVER_NAME + ", running version irc-1.0" + "\r\n")
# define RPL_YOURHOST(nickName) ((std::string)SERVER_NAME + " 003 " + nickName + " :" + "This server was created Tue Mars 23 2024 at 22:15:05 CEST" + "\r\n")
# define RPL_MYINFO(nickName) ((std::string)SERVER_NAME + " 004 " + nickName + " " + SERVER_NAME + "\r\n")

# define ERR_NONICKNAMEGIVEN ((std::string)SERVER_NAME + " 431 * :No nickname given\r\n")
# define ERR_ERRONEUSNICKNAME(nick) ((std::string)SERVER_NAME + " 432 " + nick + " :Erroneous nickname\r\n")
# define ERR_NICKNAMEINUSE(nick) ((std::string)SERVER_NAME + " 433 " + nick + " :Nickname is already in use\r\n")
# define ERR_NEEDMOREPARAMS(cmd) ((std::string)SERVER_NAME + " 461 * " + cmd + " :Not enough parameters\r\n")
# define ERR_ALREADYREGISTRED ((std::string)SERVER_NAME + " 462 * :You may not reregister\r\n")
# define ERR_PASSWDMISMATCH ((std::string)SERVER_NAME + " 464 * :Password incorrect\r\n")
# define ERR_NOTREGISTERED ((std::string)SERVER_NAME + " 451 * :You have not registered\r\n")
# define ERR_UNKNOWNCOMMAND(nick, cmd) ((std::string)SERVER_NAME + " 421 " + nick + " " + cmd + " :Unknown command\r\n")

class Server 
{
    private :
        int _port;
        std::string _password;
        int _serverSocket;
        sockaddr_in _serverAddress;
        std::map<int, Client *> _clients;
        std::map<std::string, void (Server::*)(Client *, std::vector<std::string>)> _commands;
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
        void CAP(Client *client, std::vector<std::string> tokens);
        void PASS(Client *client, std::vector<std::string> tokens);
        void NICK(Client *client, std::vector<std::string> tokens);
        void USER(Client *client, std::vector<std::string> tokens);
        void PING(Client *client, std::vector<std::string> tokens);
};


#endif