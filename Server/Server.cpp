#include "Server.hpp"

Server::Server(std::string port, std::string password)
{
    validateArgs(port, password);
    _port = std::stoi(port);
    _password = password;

    initSocket();
}

Server::~Server()
{
    close(_socket);
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        close(it->first);
    std::cout << "Server closed" << std::endl;
}

void Server::validateArgs(std::string port, std::string password)
{
    if (std::all_of(port.begin(), port.end(), ::isdigit) == false)
        throw std::invalid_argument("Port must be a number");

    if (std::stoi(port) < 1 || std::stoi(port) > 65535)
        throw std::invalid_argument("Port must be between 1 and 65535");

    if (password.length() < 1 || password.length() > 16)
        throw std::invalid_argument("Password must be between 1 and 16 characters");
}

void Server::initSocket()
{
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_socket == -1)
        throw std::runtime_error("Failed to create socket");

    if (fcntl(_socket, F_SETFL, O_NONBLOCK) < 0)
        throw std::runtime_error("Failed to set socket to non-blocking mode");

    
    int opt = 1;
    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
        throw std::runtime_error("Failed to set socket options");

    struct sockaddr_in ServerAddr;
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_addr.s_addr = INADDR_ANY;
    ServerAddr.sin_port = htons(_port);
    if (bind(_socket, (struct sockaddr *)&ServerAddr, sizeof(ServerAddr)) == -1)
        throw std::runtime_error("Failed to bind socket");
    
    if (listen(_socket, 128) == -1)
        throw std::runtime_error("Failed to listen on socket");
    
}

void Server::run()
{
    struct pollfd serverPoll;
    serverPoll.fd = _socket;
    serverPoll.events = POLLIN; // POLLIN means there is data to read
    _pollfds.push_back(serverPoll);

    std::cout << "Server is running on port " << _port << std::endl;

    while (1)
    {
        int ret = poll(_pollfds.data(), _pollfds.size(), -1);
        if (ret < 0)
        {
            if (errno == EINTR)
                continue;
            throw std::runtime_error("Poll failed: " + std::string(strerror(errno)));
        }
        
        for (size_t i = 0; i < _pollfds.size(); i++)
        {
            try
            {
                if (_pollfds[i].revents & POLLIN) 
                {
                    if (_pollfds[i].fd == _socket) // New client
                        acceptClient();

                    else // Existing client sent a message
                        receiveMessage(_pollfds[i].fd);
                }
                
                // POLLHUP means the client has disconnected.
                // POLLERR means an error occurred.
                if (_pollfds[i].revents & (POLLHUP | POLLERR))
                {
                    removeClient(_pollfds[i].fd);
                }
            }

            catch (const std::exception &e)
            {
                removeClient(_pollfds[i].fd);
            }
        }

        
    }
}


void Server::acceptClient()
{
    struct sockaddr_in ClientAddr;
    socklen_t ClientAddrSize = sizeof(ClientAddr);

    int clientSocket = accept(_socket, (struct sockaddr *)&ClientAddr, &ClientAddrSize);
    if (clientSocket == -1)
        throw std::runtime_error("Failed to accept client");
    
    if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0)
        throw std::runtime_error("Failed to set client socket to non-blocking mode");

    
    struct pollfd clientPoll;
    clientPoll.fd = clientSocket;
    clientPoll.events = POLLIN;
    _pollfds.push_back(clientPoll);

    _clients.insert(std::make_pair(clientSocket, Client(clientSocket)));
    

    std::cout << "New client connected on fd " << clientSocket << std::endl;
}

void    Server::receiveMessage(int clientSocket)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0)
        throw std::runtime_error("Failed to receive data from client");
        
    Client &client = _clients[clientSocket];
    client.appendBuffer(std::string(buffer, bytesReceived));

    size_t pos;
    while ((pos = client.getBuffer().find("\n")) != std::string::npos)
    {
        std::string cmd = client.getBuffer().substr(0, pos);
        client.setBuffer(client.getBuffer().substr(pos + 1));
        if (cmd[cmd.length() - 1] == '\r')
            cmd = cmd.substr(0, cmd.length() - 1);
        processCommand(clientSocket, cmd);
    }
}

void    Server::processCommand(int clientSocket, std::string &command)
{
    std::istringstream iss(command);
    std::string cmd, param;

    iss >> cmd;
    std::getline(iss, param);

    if (_clients[clientSocket].usingBot())
    {
        if (cmd != "1" && cmd != "2" && cmd != "3" && cmd != "4")
        {
            sendResponse(clientSocket, "\033[31m\n So you are really not that smart, You need to reply with 1, 2, 3 or 4.\n\n\033[0m");
            return;
        }
        int i = 0;
        while (param[i] && param[i] == ' ')
            i++;
        param = param.substr(i);
        _BOT(clientSocket, std::stoi(cmd), param);
        return; 
    }
    
    if (!param.empty() && param[0] == ' ')
        param = param.substr(1);
    
    if (cmd == "CAP" || cmd == "WHO")
        return;
    if(_clients[clientSocket].knowsPassword() && _clients[clientSocket].hasNickname() && _clients[clientSocket].hasUsername())
        _clients[clientSocket].setAuthenticated(true);
  
    if (cmd == "QUIT")
        _QUIT(clientSocket, param);
    else if (cmd == "PASS")
        _PASS(clientSocket, param);
    else if (cmd == "NICK" && _clients[clientSocket].knowsPassword())
        _NICK(clientSocket, param);
    else if (cmd == "USER" && _clients[clientSocket].knowsPassword())
        _USER(clientSocket, param);
    else if (!_clients[clientSocket].isAuthenticated())
        sendResponse(clientSocket, "You have not registered\n");
    else if (cmd == "JOIN")
        _JOIN(clientSocket, param);
    else if (cmd == "PRIVMSG")
        _PRIVMSG(clientSocket, param);
    else if (cmd == "TOPIC")
        _TOPIC(clientSocket, param);
    else if (cmd == "MODE")
        _MODE(clientSocket, param);
    else if (cmd == "KICK")
        _KICK(clientSocket, param);
    else if (cmd == "INVITE")
        _INVITE(clientSocket, param);
    else if (cmd == "BOT")
        _BOT(clientSocket, 0, param);
    else
        sendResponse(clientSocket, command + ": Unknown command\n");
}

void    Server::sendResponse(int clientSocket, const std::string &response)
{  
    int bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
    if (bytesSent == -1)
        throw std::runtime_error("Failed to send response to client");
}

void    Server::removeClient(int clientSocket)
{
    Client &client = _clients[clientSocket];
    
    std::set<std::string> channels = client.getChannels();
    for (std::set<std::string>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        if (_channels.find(*it) != _channels.end())
            _channels[*it].removeUser(clientSocket);
            _channels[*it].removeOperator(clientSocket);
            _channels[*it].removeInvitedUser(clientSocket);
    }
    
    if (!client.getNickname().empty())
        _nicknames.erase(client.getNickname());

    close(clientSocket);
    _clients.erase(clientSocket);

    for (std::vector<struct pollfd>::iterator it = _pollfds.begin(); it != _pollfds.end(); ++it)
    {
        if (it->fd == clientSocket)
        {
            _pollfds.erase(it);
            break;
        }
    }
    std::cout << "Client on fd " << clientSocket << " disconnected" << std::endl;
}

void    Server::broadcastToChannel(const std::string &channel, const std::string &message)
{
    if (_channels.find(channel) == _channels.end())
        return;

    const std::set<int> &users = _channels[channel].getUsers();
    for (std::set<int>::iterator it = users.begin(); it != users.end(); ++it)
    {
     sendResponse(*it, message);
    }   
}

void    Server::broadcastToFriends(int clientSocket, const std::string &message)
{
    const std::set<std::string> &channels = _clients[clientSocket].getChannels();
    std::set<int> users;
    for (std::set<std::string>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        const std::set<int> &channelUsers = _channels[*it].getUsers();
        users.insert(channelUsers.begin(), channelUsers.end());
    }
    
    for (std::set<int>::iterator it = users.begin(); it != users.end(); ++it)
    {
        if (*it != clientSocket)
            sendResponse(*it, message);
    }
}

bool    Server::isNicknameValid(const std::string &nickname)
{
    if (nickname.empty() || nickname.length() > 9)
        return false;

    if (nickname[0] == ':' || nickname[0] == '#' || nickname[0] == '&' || nickname[0] == '!')
        return false;

    if (nickname.find(' ') != std::string::npos)
        return false;
    
    return true;
}

bool    Server::isChannelValid(const std::string &channel)
{
    if (channel.empty() || channel.length() > 49)
        return false;

    if (channel[0] != '#' && channel[0] != '&')
        return false;

    if (_channels.find(channel) != _channels.end())
        return false;

    return true;
}


