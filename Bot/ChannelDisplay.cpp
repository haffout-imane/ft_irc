#include "../Channel/Channel.hpp"
#include "../Server/Server.hpp"
#include "../Client/Client.hpp"

void    Server::ChannelDisplay(int clientSocket)
{
    if (_channels.empty())
    {
        sendResponse(clientSocket, "\n❌ There are no channels available on this server.\n\n");
        _clients[clientSocket].setBot(false);
        return;
    }

    std::string message = "\n📜 Here are the channels available on this server:\n\n";
    for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        message += "\033[34m🔹 " + it->first + "\033[0m\n";
    }

    sendResponse(clientSocket, message);
    _clients[clientSocket].setBot(false);
}