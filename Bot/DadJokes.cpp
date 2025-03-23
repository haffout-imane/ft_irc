#include "../Channel/Channel.hpp"
#include "../Server/Server.hpp"
#include "../Client/Client.hpp"
#include <ctime>

void    Server::DadJoke(int clientSocket)
{
    std::string dadJokes[] = {
        "\033[32mParallel lines have so much in common. It’s a shame they’ll never meet.\033[0m",
        "\033[32mWhy can't a nose be 12 inches long? Because then it would be a foot!\033[0m",
        "\033[32mWhy don't some couples go to the gym? Because some relationships don't work out!\033[0m",
        "\033[32mWhy did the math book look sad? Because it had too many problems!\033[0m",
        "\033[32mWhat do you call a fake noodle? An impasta!\033[0m",
        "\033[32mWhy don’t eggs tell jokes? They might crack up!\033[0m",
        "\033[32mWhy do fish never do well in 1337? Because they're always swimming below \"sea\" level!\033[0m",
        "\033[32mWhat’s orange and sounds like a parrot? A carrot!\033[0m",
        "\033[32mWhy was the belt arrested? Because it was holding up some pants!\033[0m",
        "\033[32mWhat did one plate say to the other? Lunch is on me!\033[0m"
    };


    std::srand(std::time(0));
    int randomIndex = std::rand() % 10;
    sendResponse(clientSocket, "\n" + dadJokes[randomIndex] + "\n\n");
    _clients[clientSocket].setBot(false);
}