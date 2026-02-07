#include "Server.hpp"

Server::Server(std::string port, std::string password): _port(atoi(port.c_str())), _password(password), _is_running(false) 
{
    if (_port < 1024 || _port > 65535)
        throw std::invalid_argument("Bad port");
        
    for (std::string::iterator it = _password.begin(); it != _password.end(); ++it)
    {
        if (!std::isprint(*it))
            throw std::invalid_argument("Password must have only printable characters");

    }

    std::cout << htons(_port) << std::endl;
    _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket == -1)
        throw std::runtime_error(std::string("socket :") + ::strerror(errno));
        
    int flags = fcntl(_serverSocket, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error(std::string("Get flags :") + ::strerror(errno));

    if (fcntl(_serverSocket, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error(std::string("Set flags :") + ::strerror(errno));

    _serverAddress.sin_family = AF_INET;
    _serverAddress.sin_port = htons(_port);
    _serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(&_serverAddress.sin_zero, 0, sizeof(_serverAddress.sin_zero));

    const int optval = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0)
        throw std::runtime_error(std::string("setsockopt :") + ::strerror(errno));

    if (bind(_serverSocket, (const sockaddr *)&_serverAddress, sizeof(_serverAddress)) == -1)
        throw std::runtime_error(std::string("bind :") + ::strerror(errno));

    if (listen(_serverSocket, 5) == -1)
        throw std::runtime_error(std::string("listen :") + ::strerror(errno));
    
    _commands["CAP"] = &Server::CAP;
    _commands["PASS"] = &Server::PASS;
    _commands["NICK"] = &Server::NICK;
    _commands["USER"] = &Server::USER;

    std::cout << "Server listening on port " << _port << std::endl;
}

Server::~Server()
{
    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        delete(_clients[it->first]);
    }
    close(_serverSocket);
}

void Server::run()
{
    int nfds;
    struct epoll_event	ev, events[EVENTS_MAX];
    nfds = _serverSocket;
    _epollfd = epoll_create1(0);

	if (_epollfd == -1)
		throw std::runtime_error("Error: failed to create epoll");

	memset(&ev, 0, sizeof(ev));
	ev.events = EPOLLIN;
	ev.data.fd = _serverSocket;

	if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, _serverSocket, &ev) == -1) 
		throw std::runtime_error("Error: failed to manage sockets");
        
    _is_running = true;
    while (_is_running)
    {
        nfds = epoll_wait(_epollfd, events, EVENTS_MAX, -1); 
        if (nfds == -1)
            throw std::runtime_error(std::string("select :") + ::strerror(errno));
        for (int i = 0; i < nfds; ++i)
        {
            handle_event(events[i]);
        }
    }
}

void Server::handle_event(epoll_event event)
{

    if (event.data.fd == _serverSocket)
    {
        std::cout << "new client" << std::endl;
        new_client();
        return ;
    }
    if (event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
    {
        std::cout << "Client " << event.data.fd << " error/shutdown" << std::endl;
        return;
    }

    if (event.events & EPOLLOUT)
    {
        _clients[event.data.fd]->flush_send();
    }

    if (event.events & EPOLLIN)
    {
        std::cout << "fill buffer" << std::endl;
        _clients[event.data.fd]->fill_recv_buffer();
        parse_buffer(event.data.fd);
    }
}

void Server::new_client() 
{
    int client_socket;
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while ((client_socket = accept(_serverSocket, (sockaddr *)&client_addr, &addr_len)) > 0)
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
        ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
        ev.data.fd = client_socket;
        epoll_ctl(_epollfd, EPOLL_CTL_ADD, client_socket, &ev);

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

void Server::parse_msg(std::string msg, int client_socket)
{
    std::cout << msg << std::endl;
    std::string cmd = msg.substr(0, msg.find(" "));
    std::map<std::string, void (Server::*)(Client *, std::string)>::iterator it = _commands.find(cmd);
    if (it != _commands.end())
    {
        (this->*_commands[cmd.c_str()])(_clients[client_socket], msg);
        _clients[client_socket]->flush_send();
    }
}

