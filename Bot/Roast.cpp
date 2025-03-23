#include "../Channel/Channel.hpp"
#include "../Server/Server.hpp"
#include "../Client/Client.hpp"
#include <ctime>

void    Server::RoastSomeone(int clientSocket, std::string &param)
{
    if (param.empty())
    {
        sendResponse(clientSocket, "\n\033[31mYou need to provide a nickname to roast someone. re-choose an option and do it correctly its not that hard!\n\n \033[0m");
        return;
    }

    if (_nicknames.find(param) == _nicknames.end())
    {
        sendResponse(clientSocket, "\n\033[31m" + param + ": User does not exist in this server. re-choose an option and do it correctly its not that hard! \n\n\033[0m");
        return;
    }

    std::string roasts[] = {
        "\033[31mYou're like a cloud. When you disappear, it's a beautiful day.\033[0m",
        "\033[31mI’d agree with you, but then we’d both be wrong.\033[0m",
        "\033[31mYou bring everyone so much joy when you leave the room.\033[0m",
        "\033[31mYou're proof that even Google doesn't have all the answers.\033[0m",
        "\033[31mYour secrets are always safe with me. I never even listen when you tell me them.\033[0m",
        "\033[31mYou're not stupid; you just have bad luck thinking.\033[0m",
        "\033[31mIf I had a dollar for every time you said something smart, I’d be broke.\033[0m",
        "\033[31mYou're like a software update. Whenever I see you, I think, 'Not now.'\033[0m",
        "\033[31mYou have the perfect face for radio.\033[0m",
        "\033[31mYour brain is like a web browser: 19 tabs open, and none of them are relevant.\033[0m"
    };



    std::srand(std::time(0));
    int randomRoast = std::rand() % 10;
    int roastedSocket = _nicknames[param];
    sendResponse(roastedSocket, "\n" + roasts[randomRoast] + "\n\n");
    _clients[clientSocket].setBot(false);
}
