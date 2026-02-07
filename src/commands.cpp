#include "Server.hpp"

void Server::CAP(Client *client, std::string cmd)
{
    cmd.erase(0, cmd.find(" ") + 1);
    std::string response = SERVER_NAME + std::string(" CAP * LS\r\n");
    client->fill_send_buffer(response);
}

void Server::PASS(Client *client, std::string cmd)
{
    std::string response;

    if (client->get_status() != 0)
        return ;
    cmd.erase(0, cmd.find(" ") + 1);
    if (cmd == _password)
    {
        response = SERVER_NAME + std::string(" Correct password");
        client->set_status(1);
    }
    else
        response = SERVER_NAME + std::string("Incorrect password please retry");

    client->fill_send_buffer(response);
}

// void Server::NICK(Client *client, std::string cmd)
// {
// }

// void Server::USER(Client *client, std::string cmd)
// {
// }