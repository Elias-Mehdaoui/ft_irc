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
    std::vector<std::string> passwords;

    if (tokens.size() == 3)
        passwords = ft_split(tokens[2], ",");

    for (size_t i = 0; i < channels_tab.size(); i++)
    {
        if (chan_prefix.find((channels_tab[i])[0]) == std::string::npos || (channels_tab[i]).size() > 50)
        {
            client->fill_send_buffer(ERR_BADCHANNAME(channels_tab[i]));
            return ;
        }

        if (_channels.find(ft_tolower(channels_tab[i])) != _channels.end())
        {
            if (_channels[ft_tolower(channels_tab[i])]->get_key() != "")
            {
                if (i <= passwords.size())
                {
                    client->fill_send_buffer(ERR_BADCHANNELKEY(channels_tab[i]));
                    return ;
                }

                if (passwords[i] != _channels[ft_tolower(channels_tab[i])]->get_key())
                {
                    client->fill_send_buffer(ERR_BADCHANNELKEY(channels_tab[i]));
                    return ;
                }
                _channels[ft_tolower(channels_tab[i])]->new_client(client, 0);
            }
            else
                _channels[ft_tolower(channels_tab[i])]->new_client(client, 0);
            _channels[ft_tolower(channels_tab[i])]->broadcast(client, JOIN_WELCOME(client->get_nickname(), client->get_username(), client->get_host(), channels_tab[i]));

            if (_channels[ft_tolower(channels_tab[i])]->get_topic() == "")
                client->fill_send_buffer(RPL_NOTOPIC(client->get_nickname(), channels_tab[i]));
            else
                client->fill_send_buffer(RPL_TOPIC(client->get_nickname(), channels_tab[i], _channels[ft_tolower(channels_tab[i])]->get_topic()));
            
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

void char_i(Channel *channel, char sign, Client *user_target, int user_limit, std::string new_key)
{
    (void)user_target;
    (void)user_limit;
    (void)new_key;
    if (sign == '+')
        channel->set_invite_only(true);
    else
        channel->set_invite_only(false);
}

void char_t(Channel *channel, char sign, Client *user_target, int user_limit, std::string new_key)
{
    (void)user_target;
    (void)user_limit;
    (void)new_key;
    if (sign == '+')
        channel->set_topic_op_only(true);
    else
        channel->set_topic_op_only(false);
}

void char_k(Channel *channel, char sign, Client *user_target, int user_limit, std::string new_key)
{
    (void)user_target;
    (void)user_limit;
    if (sign == '+')
        channel->set_key(new_key);
    else
        channel->set_key("");
}

void char_l(Channel *channel, char sign, Client *user_target, int user_limit, std::string new_key)
{
    (void)user_target;
    (void)new_key;
    if (user_limit < 1)
        return ;
    if (sign == '+')
        channel->set_user_limit(user_limit);
    else
        channel->set_user_limit(-1);
}

void char_o(Channel *channel, char sign, Client *user_target, int user_limit, std::string new_key)
{
    (void)user_limit;
    (void)new_key;
    
    channel->set_operator(user_target, (sign == '+'));
}

void Server::MODE(Client *client, std::vector<std::string> tokens)
{
    if (tokens.size() == 1)
    {
        client->fill_send_buffer(ERR_NEEDMOREPARAMS("MODE"));
        return ;
    }

    std::string channel = tokens[1];

    if (_channels.find(ft_tolower(channel)) == _channels.end())
    {
        client->fill_send_buffer(ERR_BADCHANNAME(channel));
        return ;
    }

    if (tokens.size() == 2)
    {
        client->fill_send_buffer(RPL_CHANNELMODEIS(channel, _channels[ft_tolower(channel)]->get_modes(client)));
        return ;
    }

    if (_channels[ft_tolower(channel)]->is_operator(client) == false)
    {
        client->fill_send_buffer(ERR_CHANOPRIVSNEEDED(channel));
        return ;
    }

    std::string char_modes = tokens[2];
    char sign = char_modes[0];

    if (sign != '+' && sign != '-')
    {
        client->fill_send_buffer(ERR_UNKNOWNMODE(sign));
        return ;
    }

    if (char_modes.size() < 2)
    {
        return ;
    }

    Client *user_target = NULL;
    std::string new_key = "";
    int user_limit = -1;
    size_t j = 3;
    std::map<char, void (*)(Channel *channel, char sign, Client *user_target, int user_limit, std::string new_key)> char_exec;
    char_exec['i'] = &char_i;
    char_exec['t'] = &char_t;
    char_exec['k'] = &char_k;
    char_exec['o'] = &char_o;
    char_exec['l'] = &char_l;

        for (size_t i = 1; i < char_modes.size(); i++)
    {

        if (char_modes[i] == 'o')
        {
            if (tokens.size() < j + 1)
            {
                client->fill_send_buffer(ERR_NEEDMOREPARAMS("MODE"));
                return ;
            }

            user_target = this->get_client(tokens[j]);

            if (user_target == NULL)
            {
                client->fill_send_buffer(ERR_NOSUCHNICK(tokens[j]));
                return ;
            }

            if (_channels[ft_tolower(channel)]->is_in_chan(user_target) == false)
            {
                client->fill_send_buffer(ERR_USERNOTINCHANNEL(tokens[j], channel));
                return ;
            }

            j++;
        }

        if ( sign == '+' && (char_modes[i] == 'l' || char_modes[i] == 'k'))
        {
            if (tokens.size() < j + 1)
            {
                client->fill_send_buffer(ERR_NEEDMOREPARAMS("MODE"));
                return ;
            }
            if (char_modes[i] == 'l')
                user_limit = atoi(tokens[j].c_str());
            else
                new_key = tokens[j];
            j++;
        }
    }
    for (size_t i = 1; i < char_modes.size(); i++)
    {
        char_exec[char_modes[i]](_channels[ft_tolower(channel)], sign, user_target, user_limit, new_key);
    }
    _channels[ft_tolower(channel)]->broadcast(client, RPL_CHANNELMODEIS(channel, _channels[ft_tolower(channel)]->get_modes(client)));
}
