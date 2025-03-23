#include "../Channel/Channel.hpp"
#include "../Server/Server.hpp"
#include "../Client/Client.hpp"

void    Server::_BOT(int clientSocket, int option, std::string &param)
{
    std::string botMessage = "";
    switch (option)
    {
        case 0:
            botMessage =
            "\n🤖 Hey there, human! I’m your friendly (or not-so-friendly) bot. What do you want me to do?\n\n"
            "1️⃣   See all channel names – Need a map of this wild server? I got you.\n\n"
            "2️⃣   Roast someone anonymously – Give me a nickname, and I’ll obliterate them with words. (No one will know it was you… 😉)\n\n"
            "3️⃣   Get a dad joke – Because who doesn’t love cringy dad humor?\n\n"
            "4️⃣   Exit the bot – If you’re tired of me, I’ll leave you alone. 😢\n\n"
            "🔢   Reply with 1, 2, or 3 to choose: ";

            sendResponse(clientSocket, botMessage);
            _clients[clientSocket].setBot(true);
            break;

        case 1:
            ChannelDisplay(clientSocket);
            break;

        case 2:
            RoastSomeone(clientSocket, param);
            break;

        case 3:
            DadJoke(clientSocket);
            break;
        
        case 4:
            sendResponse(clientSocket, "You have chosen to exit the bot. Goodbye!\n");
            break;
    }
}