#include "./Server/Server.hpp"

int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cerr << "Usage: ./ircserv [port] [password]" << std::endl;
        return 1;
    }
    
    Server server(av[1], av[2]);

    try
    {
       server.run();
       return 0; 
    }

    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}