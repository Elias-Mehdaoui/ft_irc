# ifndef SERVER_HPP
# define SERVER_HPP


#include "Client.hpp"
#include "Channel.hpp"

# define EVENTS_MAX 10
# define SERVER_NAME ":irc.serv.42" 

# define RPL_WELCOME(nick_name) ((std::string)SERVER_NAME + " 001 " + nick_name + " :Welcome to my irc server, " + nick_name)
# define RPL_YOURID(nick_name) ((std::string)SERVER_NAME + " 002 " + nick_name + " :" + "Your host is " + SERVER_NAME + ", running version irc-1.0")
# define RPL_YOURHOST(nick_name) ((std::string)SERVER_NAME + " 003 " + nick_name + " :" + "This server was created Tue Mars 23 2024 at 22:15:05 CEST")
# define RPL_MYINFO(nick_name) ((std::string)SERVER_NAME + " 004 " + nick_name + " " + SERVER_NAME)

# define RPL_CHANNELMODEIS(channel, modes) ((std::string)SERVER_NAME + " 324 * " + channel + " " + modes)

# define RPL_NOTOPIC(nick, channel) ((std::string)SERVER_NAME + " 331 " + nick + " " + channel + " :No topic is set")
# define RPL_TOPIC(nick, channel, topic) ((std::string)SERVER_NAME + " 332 " + nick + " " + channel + " " + topic)

# define RPL_NAMREPLY(nick, channel, names) ((std::string)SERVER_NAME + " 353 " + nick + " = " + channel + " :" + names)
# define RPL_ENDOFNAMES(nick, channel) ((std::string)SERVER_NAME + " 366 " + nick + " " + channel + " :End of NAMES list")

# define CLIENT_PREFIX(nick, user, host) ((std::string)":" + nick + "!" + user + "@" + host)
# define JOIN_WELCOME(nick, user, host, channel) (CLIENT_PREFIX(nick, user, host) + " JOIN :" + channel)
# define CLIENT_PRIVMSG(nick, user, host, target, msg) (CLIENT_PREFIX(nick, user, host) + " PRIVMSG " + target + " " + msg)
# define NEW_TOPIC(nick, user, host, channel, topic) (CLIENT_PREFIX(nick, user, host) + " TOPIC " + channel + " " + topic)
# define CHANGE_MODE(nick, user, host, channel, msg) (CLIENT_PREFIX(nick, user, host) + " MODE " + channel + " " + msg)

# define RPL_KICK(nick, user, host, channel, target, comment) (CLIENT_PREFIX(nick, user, host)  + " KICK " + channel + " " + target + " " + comment)
# define RPL_INVITE(nick, user, host, target, channel) (CLIENT_PREFIX(nick, user, host) + " INVITE " + target + " :" + channel)
# define RPL_INVITING(inviterNick, targetNick, channel) ((std::string)SERVER_NAME + " 341 " + inviterNick + " " + targetNick + " " + channel + " :Inviting")
# define RPL_PART(nick, user, host, channel) (CLIENT_PREFIX(nick, user, host) + " PART " + channel)
# define RPL_PART_MSG(nick, user, host, channel, message) (CLIENT_PREFIX(nick, user, host) + " PART " + channel + " " + message)
# define RPL_QUIT(nick, user, host, message) (CLIENT_PREFIX(nick, user, host) + " QUIT " + message)
# define RPL_ERROR(message) ((std::string)"ERROR " + message)


# define ERR_CHANNELISFULL(clientNick, channel) ((std::string)SERVER_NAME + " 471 " + clientNick + " " + channel + " :Cannot join channel (+l)")
# define ERR_NONICKNAMEGIVEN ((std::string)SERVER_NAME + " 431 * :No nickname given")
# define ERR_ERRONEUSNICKNAME(nick) ((std::string)SERVER_NAME + " 432 " + nick + " :Erroneous nickname")
# define ERR_NICKNAMEINUSE(nick) ((std::string)SERVER_NAME + " 433 " + nick + " :Nickname is already in use")
# define ERR_NEEDMOREPARAMS(cmd) ((std::string)SERVER_NAME + " 461 * " + cmd + " :Not enough parameters")
# define ERR_ALREADYREGISTRED ((std::string)SERVER_NAME + " 462 * :You may not reregister")
# define ERR_PASSWDMISMATCH ((std::string)SERVER_NAME + " 464 * :Password incorrect")
# define ERR_NOTREGISTERED ((std::string)SERVER_NAME + " 451 * :You have not registered")
# define ERR_UNKNOWNCOMMAND(nick, cmd) ((std::string)SERVER_NAME + " 421 " + nick + " " + cmd + " :Unknown command")
# define ERR_BADCHANNAME(channel) ((std::string)SERVER_NAME + " 479 " + channel + " :Bad channel name")
# define ERR_NORECIPIENT(cmd) ((std::string)SERVER_NAME + " 411 * :No recipient given (" + cmd + ")")
# define ERR_NOTEXTTOSEND ((std::string)SERVER_NAME + " 412 * :No text to send")
# define ERR_NOSUCHNICK(nick, target) ((std::string)SERVER_NAME + " 401 " + nick + " " + target + " :No such nick/channel")
# define ERR_CHANOPRIVSNEEDED(channel) ((std::string)SERVER_NAME + " 482 " + channel + " :You're not channel operator")
# define ERR_UNKNOWNMODE(modechar)((std::string)SERVER_NAME + " 472 " + modechar + " :is unknown mode char")
# define ERR_USERNOTINCHANNEL(user, channel) ((std::string)SERVER_NAME + " 441 " + user + " " + channel + " :They aren't on that channel")
# define ERR_NOTONCHANNEL(nick, channel) ((std::string)SERVER_NAME + " 442 " + nick + " " + channel + " :You're not on that channel")
# define ERR_BADCHANNELKEY(channel) ((std::string)SERVER_NAME + " 475 * " + channel + " :Cannot join channel (+k)")
# define ERR_NOSUCHCHANNEL(nick, channel) ((std::string)SERVER_NAME + " 403 " + nick + " " + channel + " :No such channel")
# define ERR_USERONCHANNEL(senderNick, targetNick, channel) ((std::string)SERVER_NAME + " 443 " + senderNick + " " + targetNick + " " + channel + " :is already on channel")
# define ERR_INVITEONLYCHAN(client, channel) ((std::string)SERVER_NAME + " 473 " + client + " " + channel + " :Cannot join channel (+i)")

class Server 
{
    private :
        int _port;
        int _epollfd;
        bool _is_running;
        int _server_socket;
        std::string _password;
        sockaddr_in _server_address;

        std::map<int, Client *> _clients;
        std::map<std::string, Channel *> _channels;
        std::map<std::string, void (Server::*)(Client *, std::vector<std::string>)> _commands;

    
    public :
        Server(std::string port, std::string password);
        ~Server();

        void run();
        void handle_event(struct epoll_event event);
        void new_client();
        void fill_buffer(int client_socket);
        void parse_buffer(int client_socket);
        void parse_msg(std::string msg, int client_socket);
        Client *get_client(std::string client_name);
        Channel *get_channel(std::string channel);
        void handle_quit(Client* client, std::string msg);

        // Commands
        void CAP(Client *client, std::vector<std::string> tokens);
        void PASS(Client *client, std::vector<std::string> tokens);
        void NICK(Client *client, std::vector<std::string> tokens);
        void USER(Client *client, std::vector<std::string> tokens);
        void PING(Client *client, std::vector<std::string> tokens);
        void JOIN(Client *client, std::vector<std::string> tokens);
        void PRIVMSG(Client *client, std::vector<std::string> tokens);
        void MODE(Client *client, std::vector<std::string> tokens);
        void TOPIC(Client *client, std::vector<std::string> tokens);
        void KICK(Client *client, std::vector<std::string> tokens);
        void INVITE(Client *client, std::vector<std::string> tokens);
        void PART(Client *client, std::vector<std::string> tokens);
        void QUIT(Client *client, std::vector<std::string> tokens);
};


#endif