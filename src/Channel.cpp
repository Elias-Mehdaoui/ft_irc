#include "Channel.hpp"

Channel::Channel(Client *client, std::string name)
{
    _clients[client] = true;
    _name = name;
}

Channel::~Channel()
{

}

void Channel::new_client(Client *client, bool is_operator)
{
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
            it->first->flush_send();
        }
    }
}