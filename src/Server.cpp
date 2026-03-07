#include "Server.hpp"

Server::Server(std::string port, std::string password)
{
    _port = atoi(port.c_str());
    _password = password;
    _is_running = false;


    if (_port < 1024 || _port > 65535)
        throw std::invalid_argument("Bad port");
        
    for (std::string::iterator it = _password.begin(); it != _password.end(); ++it)
    {
        if (!std::isprint(*it))
            throw std::invalid_argument("Password must have only printable characters");

    }

    _server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_socket == -1)
        throw std::runtime_error(std::string("socket :") + ::strerror(errno));
        
    int flags = fcntl(_server_socket, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error(std::string("Get flags :") + ::strerror(errno));

    if (fcntl(_server_socket, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error(std::string("Set flags :") + ::strerror(errno));

    _server_address.sin_family = AF_INET;
    _server_address.sin_port = htons(_port);
    _server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(&_server_address.sin_zero, 0, sizeof(_server_address.sin_zero));

    const int optval = 1;
    if (setsockopt(_server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0)
        throw std::runtime_error(std::string("setsockopt :") + ::strerror(errno));

    if (bind(_server_socket, (const sockaddr *)&_server_address, sizeof(_server_address)) == -1)
        throw std::runtime_error(std::string("bind :") + ::strerror(errno));

    if (listen(_server_socket, 5) == -1)
        throw std::runtime_error(std::string("listen :") + ::strerror(errno));
    
    _commands["CAP"] = &Server::CAP;
    _commands["PASS"] = &Server::PASS;
    _commands["NICK"] = &Server::NICK;
    _commands["USER"] = &Server::USER;
    _commands["PING"] = &Server::PING;
    _commands["JOIN"] = &Server::JOIN;
    _commands["PRIVMSG"] = &Server::PRIVMSG;
    _commands["MODE"] = &Server::MODE;
    _commands["TOPIC"] = &Server::TOPIC;
    _commands["KICK"] = &Server::KICK;
    _commands["INVITE"] = &Server::INVITE;

    std::cout << "Server listening on port " << _port << std::endl;
}

Server::~Server()
{
    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        delete it->second;
    }
    close(_server_socket);
    close(_epollfd);
}

void Server::run()
{
    int nfds;
    struct epoll_event	ev, events[EVENTS_MAX];
    nfds = _server_socket;
    _epollfd = epoll_create1(0);

	if (_epollfd == -1)
		throw std::runtime_error("Error: failed to create epoll");

	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = _server_socket;

	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, _server_socket, &ev) == -1) 
		throw std::runtime_error("Error: failed to manage sockets");
        
    _is_running = true;
    while (_is_running && !g_end)
    {
        nfds = epoll_wait(_epollfd, events, EVENTS_MAX, -1); 
        if (nfds == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            throw std::runtime_error(std::string("select :") + ::strerror(errno));
        }
        for (int i = 0; i < nfds; ++i)
        {
            handle_event(events[i]);
        }
    }
}

void Server::handle_event(epoll_event event)
{

    if (event.data.fd == _server_socket)
    {
        new_client();
        return ;
    }

    if (event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
    {
        std::cout << "Client " << event.data.fd << " error/shutdown" << std::endl;
        delete _clients[event.data.fd];
        close(event.data.fd);
        return;
    }
    
    if (event.events & EPOLLIN)
    {
        _clients[event.data.fd]->fill_recv_buffer();
        parse_buffer(event.data.fd);
    }

    if (event.events & EPOLLOUT)
    {
        _clients[event.data.fd]->flush_send();
    }

}

void Server::new_client() 
{
    int client_socket;
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while ((client_socket = accept(_server_socket, (sockaddr *)&client_addr, &addr_len)) > 0)
    {
        if (client_socket == -1)
        {
            if (errno == EAGAIN && errno == EWOULDBLOCK)
            {
                std::cout << "No more pending connection" << std::endl;
                break;
            }
            throw std::runtime_error(std::string("client socket:") + ::strerror(errno));
        }

        int flags = fcntl(client_socket, F_GETFL, 0);
        if (flags == -1)
            throw std::runtime_error(std::string("Get flags :") + ::strerror(errno));

        if (fcntl(client_socket, F_SETFL, flags | O_NONBLOCK) == -1)
            throw std::runtime_error(std::string("Set flags :") + ::strerror(errno));

        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLRDHUP | EPOLLOUT;
        ev.data.fd = client_socket;
        if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, client_socket, &ev) == -1)
            throw std::runtime_error(std::string("epoll_ctl: ") + ::strerror(errno));

        _clients[client_socket] = new Client(client_socket, client_addr);
        std::cout << "New connection from client id : " << client_socket << std::endl;
        _clients[client_socket]->fill_recv_buffer();
        parse_buffer(client_socket);
    }
} 

void Server::parse_buffer(int client_socket)
{
    size_t pos;
    std::string msg;
    std::string recv_buffer = (std::string)_clients[client_socket]->get_recv_buff(); 
    while ((pos = recv_buffer.find("\r\n")) != std::string::npos)
    {
        msg = recv_buffer.substr(0, pos);
        recv_buffer.erase(0, pos + 2);

        if (!msg.empty())
            parse_msg(msg, client_socket);
    }
    _clients[client_socket]->clear_recv_buff();
}

bool is_registration_cmd(std::string cmd) {
    return cmd == "CAP" || cmd == "PASS" || cmd == "NICK" || cmd == "USER";
}

void Server::parse_msg(std::string msg, int client_socket)
{
    std::vector<std::string> tokens = ft_split(msg, " ");

    std::cout << "Receiving :" << msg << std::endl;

    std::map<std::string, void (Server::*)(Client *, std::vector<std::string>)>::iterator it = _commands.find(tokens[0]);

    if (it != _commands.end())
    {
        if (is_registration_cmd(tokens[0]) == false && _clients[client_socket]->is_registered() == false)
        {
            _clients[client_socket]->fill_send_buffer(ERR_NOTREGISTERED);
        }
        else
        {
            (this->*_commands[tokens[0]])(_clients[client_socket], tokens);
        }
    }
    else
        _clients[client_socket]->fill_send_buffer(ERR_UNKNOWNCOMMAND(_clients[client_socket]->get_nickname(), tokens[0]));
}

Client* Server::get_client(std::string client_name)
{
    std::map<int, Client *>::iterator it;

    for (it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second->get_nickname() == client_name)
            return it->second;
    }
    return NULL;
}

Channel* Server::get_channel(std::string channel)
{

    if (_channels.find(ft_tolower(channel)) == _channels.end())
        return NULL;
    return (_channels[ft_tolower(channel)]);
}
