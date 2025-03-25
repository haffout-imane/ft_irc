#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <string>
#include <exception>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include <poll.h>
#include <algorithm>
#include <map>
#include <sstream>

#include "../Client/Client.hpp"
#include "../Channel/Channel.hpp"

class Client;
class Channel;

class Server
{
    private:
        int _port;
        std::string _password;
        int _socket;
        
        std::vector<struct pollfd> _pollfds;
        std::map<int, Client> _clients;
        std::map<std::string, Channel> _channels;
        std::map<std::string, int> _nicknames;
        
        

        Server();
        Server(Server const &src);
        Server &operator=(Server const &src);


        void    _PASS(int fd, const std::string &param);
        void    _NICK(int fd, const std::string &param);
        void    _USER(int fd, const std::string &param);
        void    _JOIN(int fd, const std::string &param);
        void    _INVITE(int fd, const std::string &param);
        void    _PRIVMSG(int fd, const std::string &param);
        void    _TOPIC(int fd, const std::string &param);
        void    _MODE(int fd, const std::string &param);
        void    _KICK(int fd, const std::string &param);
        void    _QUIT(int fd, const std::string &param);

        
        void    broadcastToChannel(const std::string &channel, const std::string &message);
        void    broadcastToFriends(int clientSocket, const std::string &message);
        bool    isNicknameValid(const std::string &nickname);
        bool    isChannelValid(const std::string &channel);
        
        
    public:
        Server(std::string port, std::string password);
        ~Server();

        
        void validateArgs(std::string port, std::string password);

        
        void    run();
        void    initSocket();
        void    acceptClient();
        void    receiveMessage(int fd);
        void    removeClient(int fd);
        void    processCommand(int clientSocket, std::string &cmd);
        void    sendResponse(int clientSocket, const std::string &response);

        // BOT
        void    _BOT(int clientSocket, int option, std::string &param);
        void    ChannelDisplay(int clientSocket);
        void    RoastSomeone(int clientSocket, std::string &nickname);
        void    DadJoke(int clientSocket);

};


#endif