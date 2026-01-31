#include "Server.hpp"


int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout << "Error: Usage ./ircserver <port> <password>" << std::endl;
        return (0);
    }

    try 
    {
        Server server(argv[1], argv[2]);
        server.run();
    } 
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }

}