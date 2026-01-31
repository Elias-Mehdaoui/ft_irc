#include "Server.hpp"

Server::Server(std::string port, std::string password): _port(atoi(port.c_str())), _password(password), _is_running(false) 
{
    if (_port < 1024 || _port > 65535)
        throw std::invalid_argument("Bad port");
        
    for (std::string::iterator it = _password.begin(); it != _password.end(); ++it)
    {
        if (!std::isprint(*it))
            throw std::invalid_argument("Password must have only printable characters");
;
    }

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

    if (bind(_serverSocket, (const sockaddr *)&_serverAddress.sin_addr, sizeof(_serverAddress)) == -1)
        throw std::runtime_error(std::string("bind :") + ::strerror(errno));

    if (listen(_serverSocket, 5) == -1)
        throw std::runtime_error(std::string("listen :") + ::strerror(errno));

    std::cout << "Server listening on port " << _port << std::endl;
}

Server::~Server()
{}

void Server::run()
{
    int max_fd, new_socket;
    fd_set readfds, readfds_copy;
    struct sockaddr_in client;
    socklen_t addr_len = sizeof(client);
    FD_ZERO(&readfds);
    FD_SET(_serverSocket, &readfds);
    _is_running = true;

    while (_is_running)
    {
        std::cout << "Debut while" << std::endl;
        readfds_copy = readfds;
        if (select(max_fd + 1, &readfds_copy, NULL, NULL, NULL) == -1)
            throw std::runtime_error(std::string("select :") + ::strerror(errno));
        std::cout << "Apres select" << std::endl;
        if (FD_ISSET(_serverSocket, &readfds))
        {
            std::cout << "ISSET" << std::endl;
            new_socket = accept(_serverSocket, (sockaddr *)&client, &addr_len);
            if (new_socket == -1)
                throw std::runtime_error(std::string("accept :") + ::strerror(errno));
            _clients[new_socket] = new Client(new_socket, client);
            FD_SET(new_socket, &readfds);
            if (new_socket > max_fd)
                max_fd = new_socket;
            std::cout << "New connection from " << inet_ntoa(client.sin_addr) << std::endl;
        }
    }



    


}