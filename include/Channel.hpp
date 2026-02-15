#include "Client.hpp"


class Channel 
{
    private :
        std::string _name;
        std::string _topic;
        std::map<Client *, bool> _clients;

    public :
        Channel(Client *client, std::string name);
        ~Channel();

        void broadcast(Client *sender, std::string msg);
        void new_client(Client *client, bool is_operator);



};