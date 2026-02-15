#include "Server.hpp"

bool	g_end;

void handleSignal(int signum) 
{
	if (signum == SIGINT)
		g_end = true;
}


int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout << "Error: Usage ./ircserver <port> <password>" << std::endl;
        return (0);
    }

    try 
    {
        signal(SIGINT, handleSignal);
        Server server(argv[1], argv[2]);
        server.run();
    } 
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    
}