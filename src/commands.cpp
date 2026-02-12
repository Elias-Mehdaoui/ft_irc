#include "Server.hpp"
#include <cctype>

void Server::CAP(Client *client, std::vector<std::string> tokens)
{
    (void)tokens;
    std::string response = SERVER_NAME + std::string(" CAP * LS\r\n");
    client->fill_send_buffer(response);
}

void Server::PASS(Client *client, std::vector<std::string> tokens)
{
    if (client->get_status() >= 1)
    {
        client->fill_send_buffer(ERR_ALREADYREGISTRED);
        return ;
    }
    if (tokens.size() < 2)
    {
        client->fill_send_buffer(ERR_NEEDMOREPARAMS("PASS"));
        return ;
    }
    if (tokens[1] != _password)
    {
        client->fill_send_buffer(ERR_PASSWDMISMATCH);
        return ;
    }
    client->set_status(1);
}

bool is_valid_nick_char(char c)
{
    if (std::isalnum(c))
        return true;
    return c == '-' || c == '[' || c == ']' || c == '\\' || c == '`' || c == '^' || c == '{' || c == '|' || c == '}';
}

bool is_valid_nickname(std::string nick)
{
    if (nick.size() > 9)
        return false;
    if (std::isdigit(nick[0]) || nick[0] == '-' || nick[0] == '/')
        return false;
    for (size_t i = 0; i < nick.size(); ++i)
    {
        if (!is_valid_nick_char(nick[i]))
            return false;
    }
    return true;
}

void Server::NICK(Client *client, std::vector<std::string> tokens)
{
    std::string nick;

    if (client->get_status() == 0)
    {
        client->fill_send_buffer(ERR_NOTREGISTERED);
        return ;
    }
    if (tokens.size() < 2)
    {
        client->fill_send_buffer(ERR_NONICKNAMEGIVEN);
        return ;
    }
    nick = tokens[1];
    if (!is_valid_nickname(nick))
    {
        client->fill_send_buffer(ERR_ERRONEUSNICKNAME(nick));
        return ;
    }
    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second != client && it->second->get_nickname() == nick)
        {
            client->fill_send_buffer(ERR_NICKNAMEINUSE(nick));
            return ;
        }
    }
    client->set_nick(nick);
    if (client->get_status() == 1)
        client->set_status(2);
}

void Server::USER(Client *client, std::vector<std::string> tokens)
{

    if (client->get_status() < 2)
    {
        client->fill_send_buffer(ERR_NOTREGISTERED);
        return ;
    }
    if (client->get_status() == 3)
    {
        client->fill_send_buffer(ERR_ALREADYREGISTRED);
        return ;
    }
    if (tokens.size() < 4)
    {
        client->fill_send_buffer(ERR_NEEDMOREPARAMS("USER"));
        return ;
    }

    client->set_user(tokens[1]);
    client->set_status(3);
    client->fill_send_buffer(RPL_WELCOME(client->get_nickname()));
    client->fill_send_buffer(RPL_YOURID(client->get_nickname()));
    client->fill_send_buffer(RPL_YOURHOST(client->get_nickname()));
    client->fill_send_buffer(RPL_MYINFO(client->get_nickname()));
}

void Server::PING(Client *client, std::vector<std::string> tokens)
{
    if (tokens.size() < 2)
        return ;
    client->fill_send_buffer("PONG " + tokens[1] + "\r\n");
}