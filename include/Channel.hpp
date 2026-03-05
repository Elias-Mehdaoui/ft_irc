#include "Client.hpp"


class Channel 
{
    private :
        std::string _name;
        std::map<Client *, bool> _clients;
        std::vector<Client *> _invited;
        
        std::string _key;
        std::string _topic;

        bool _topic_op_only;
        bool _invite_only;
        int _user_limit;



    public :
        Channel(Client *client, std::string name);
        ~Channel();

        void broadcast(Client *sender, std::string msg);
        void new_client(Client *client, bool is_operator);
        bool is_operator(Client *client);
        bool is_in_chan(Client *client);
        

        std::string get_topic();
        void set_topic(std::string topic);

        std::string get_key();
        void set_key(std::string key);
        
        bool get_topic_op_only();
        void set_topic_op_only(bool topic_op_only);
        
        bool get_invite_only();
        void set_invite_only(bool invite_only);
        
        int get_user_limit();
        void set_user_limit(int user_limit);

        std::string get_modes(Client *sender);
        void set_operator(Client *sender, bool status);

        std::string get_name();
};