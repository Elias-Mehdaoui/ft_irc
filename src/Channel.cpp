#include "Channel.hpp"

Channel::Channel(Client *client, std::string name)
{
    _clients[client] = true;
    _name = name;
    _key = "";
    _topic = "";
    _topic_op_only = false;
    _invite_only = false;
    _user_limit = -1;
}

Channel::~Channel()
{

}

void Channel::new_client(Client *client, bool is_operator)
{
    if (_clients.empty())
        is_operator = true;
    _clients[client] = is_operator;
}



void Channel::broadcast(Client *sender, std::string msg)
{
    std::map<Client *, bool>::iterator it;
    
    for (it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->first != sender)
        {
            it->first->fill_send_buffer(msg);
        }
    }
}

std::map<Client *, bool> Channel::get_clients()
{
    return _clients;
}


std::string Channel::get_name()
{
    return _name;
}


bool Channel::is_in_chan(Client *client)
{
    return (_clients.find(client) != _clients.end());
}


bool Channel::is_operator(Client *sender)
{
    return _clients[sender];
}

void Channel::set_operator(Client *sender, bool status)
{
    _clients[sender] = status;
}

std::string Channel::get_topic()
{
    return _topic;
}

void Channel::set_topic(std::string topic)
{
    _topic = topic;
}

std::string Channel::get_key()
{
    return _key;
}

void Channel::set_key(std::string key)
{
    _key = key;
}

bool Channel::get_topic_op_only()
{
    return _topic_op_only;
}

void Channel::set_topic_op_only(bool topic_op_only)
{
    _topic_op_only = topic_op_only;
}

bool Channel::get_invite_only()
{
    return _invite_only;
}

void Channel::set_invite_only(bool invite_only)
{
    _invite_only = invite_only;
}

int Channel::get_user_limit()
{
    return _user_limit;
}

void Channel::set_user_limit(int user_limit)
{
    _user_limit = user_limit;
}

int Channel::get_user_size()
{
    return _invited.size();
}


std::string Channel::get_modes(Client *sender)
{
    std::string modes = "+";

    if (_invite_only)
        modes += "i";
    if (_topic_op_only)
        modes += "t";
    if (_user_limit)
        modes += "l";
    if (_key != "")
        modes += "k";
    
    if (_user_limit)
        modes += " " + ft_to_string(_user_limit);
    
    if (_key != "" && this->is_in_chan(sender))
        modes += " " + _key;

    return modes;
}


void Channel::kick_client(Client *client)
{
    std::vector<Client *>::iterator it;
    for (it = _invited.begin(); it != _invited.end(); ++it)
    {
        if (*it == client)
        {
            _invited.erase(it);
        }
    }
    
    _clients.erase(client);

    std::map<Client *, bool>::iterator it_op;
    int cnt = 0;

    for (it_op = _clients.begin(); it_op != _clients.end(); ++it_op)
    {
        if (it_op->second == true)
        {
            cnt++;
        }
    }
    

    if (cnt == 0 && !_clients.empty())
        _clients.begin()->second = true;
        

}


void Channel::invite(Client *client)
{
    std::vector<Client *>::iterator it;
    for (it = _invited.begin(); it != _invited.end(); ++it)
    {
        if (*it == client)
            return ;
    }

    _invited.push_back(client);
}

bool Channel::is_invited(Client *client)
{
    std::vector<Client *>::iterator it;

    for (it = _invited.begin(); it != _invited.end(); ++it)
    {
        if (*it == client)
            return true;
    }
    return false;
}
