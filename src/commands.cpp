#include "Server.hpp"

void Server::CAP(Client *client, std::vector<std::string> tokens)
{
    (void)tokens;
    std::string response = SERVER_NAME + std::string(" CAP * LS");
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
    client->fill_send_buffer("PONG " + tokens[1]);
}

void Server::JOIN(Client *client, std::vector<std::string> tokens)
{
    if (tokens.size() < 2)
    {
        client->fill_send_buffer(ERR_NEEDMOREPARAMS("JOIN"));
        return ;
    }

    std::string chan_prefix = "#+&!";
    std::vector<std::string> channels_tab = ft_split(tokens[1], ",");

    if (tokens.size() == 3)
        std::vector<std::string> passwords = ft_split(tokens[2], ",");

    for (size_t i = 0; i < channels_tab.size(); i++)
    {
        if (chan_prefix.find((channels_tab[i])[0]) == std::string::npos || (channels_tab[i]).size() > 50)
        {
            client->fill_send_buffer(ERR_BADCHANNAME(channels_tab[i]));
            return ;
        }

        if (_channels.find(ft_tolower(channels_tab[i])) != _channels.end())
        {
            _channels[ft_tolower(channels_tab[i])]->new_client(client, 0); // verif password et tt
        }
        else
        {
            _channels[ft_tolower(channels_tab[i])] = new Channel(client, channels_tab[i]);
        }
    }
}

void Server::PRIVMSG(Client *client, std::vector<std::string> tokens)
{
    if (tokens.size() == 1)
    {
        client->fill_send_buffer(ERR_NORECIPIENT("PRIVMSG"));
        return ;
    }
    
    if (tokens.size() == 2)
    {
        client->fill_send_buffer(ERR_NOTEXTTOSEND);
        return ; 
    }

    std::string target = tokens[1];
    std::string chan_prefix = "#+&!";
    std::string msg = "";

    for (size_t i = 2; i < tokens.size(); i++)
    {
        msg += tokens[i];
        if (i != tokens.size() - 1)
            msg += " ";
    }

    if (chan_prefix.find(target[0]) != std::string::npos)
    {
        if (_channels.find(ft_tolower(target)) != _channels.end())
        {
            _channels[ft_tolower(target)]->broadcast(client, CLIENT_PRIVMSG(client->get_nickname(), client->get_username(), client->get_host(), target, msg));
        }
        else
            client->fill_send_buffer(ERR_NOSUCHNICK(target));
    }
    else
    {
        std::map<int, Client *>::iterator it;
        for (it = _clients.begin(); it != _clients.end(); ++it)
        {
            if (it->second->get_nickname() == target)
            {
                it->second->fill_send_buffer(CLIENT_PRIVMSG(client->get_nickname(), client->get_username(), client->get_host(), target, msg));
                return ;
            }
        }
        client->fill_send_buffer(ERR_NOSUCHNICK(target));
    }
}

void mode_i(Channel *channel, char sign)
{
    if (sign == '+')
        channel->set_invite_only(true);
    else
        channel->set_invite_only(false);
}

// void mode_t(Channel *channel, char sign)
// {
//     if (sign == '+')
//         channel->set_topic_op_only(true);
//     else
//         channel->set_topic_op_only(false);
// }

// void mode_k()


// void mode_o()
// {

// }


void Server::MODE(Client *client, std::vector<std::string> tokens)
{
    if (tokens.size() == 1)
    {
        client->fill_send_buffer(ERR_NEEDMOREPARAMS("MODE"));
        return ;
    }

    if (_channels.find(tokens[1]) == _channels.end())
        client->fill_send_buffer(ERR_BADCHANNAME(tokens[1]));

    if (tokens.size() == 2)
    {
        client->fill_send_buffer(RPL_CHANNELMODEIS(tokens[1], _channels[tokens[1]]->get_modes(client)));
        return ;
    }

    if (_channels[tokens[1]]->is_operator(client) == false)
    {
        client->fill_send_buffer(ERR_CHANOPRIVSNEEDED(tokens[1]));
        return ;
    }

    std::string char_modes = tokens[2];

    if (char_modes[0] != '+' || char_modes[0] != '-')
    {
        client->fill_send_buffer(ERR_UNKNOWNMODE(char_modes[0]));
    }


    if (char_modes.size() < 2)
    {
        return ;
    }

    // for (size_t i = 1; i < char_modes.size(); i++)
    // {

    //     if (char_modes[i] == 'o')
    //     {
    //         if (char_modes[0] == '+')
    //     }


    // }
}
