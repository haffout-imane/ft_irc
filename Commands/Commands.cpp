#include "../Server/Server.hpp"


void    Server::_PASS(int clientSocket, const std::string &param)
{
    if (_clients[clientSocket].isAuthenticated())
    {
        sendResponse(clientSocket, "You may not register\n");
        return;
    }

    if (param.empty())
    {
        sendResponse(clientSocket, "Not enough parameters\n");
        return;
    }

    if (param != _password)
    {
        sendResponse(clientSocket, "Password incorrect\n");
        _clients[clientSocket].setKnowsPassword(false);
        return;
    }
    _clients[clientSocket].setKnowsPassword(true);
}

void    Server::_NICK(int clientSocket, const std::string &param)
{
    if (param.empty())
    {
        sendResponse(clientSocket, "No nickname given\n");
        return;
    }

    if (_nicknames.find(param) != _nicknames.end())
    {
        sendResponse(clientSocket, param + ": Nickname is already in use\n");
        return;
    }
    
    if (!isNicknameValid(param))
    {
        sendResponse(clientSocket, param + ": Erroneus nickname\n");
        return;
    }

    std::string oldNickname = _clients[clientSocket].getNickname();
    if (!oldNickname.empty())
        _nicknames.erase(oldNickname);

    _clients[clientSocket].setNickname(param);
    _nicknames.insert(std::make_pair(param, clientSocket));

    if (_clients[clientSocket].isAuthenticated())
    {
        std::string notif = ":" + oldNickname + " NICK " + param + "\n";
        sendResponse(clientSocket, notif);
        broadcastToFriends(clientSocket, notif);
    }
    _clients[clientSocket].setHasNickname(true);
}

void    Server::_USER(int clientSocket, const std::string &param)
{
    //USER <username> <hostname> <servername> :<realname>
    if(_clients[clientSocket].isAuthenticated())
    {
        sendResponse(clientSocket, "You may not register\n");
        return;
    }
    
    // split the param into 4 parts
    std::istringstream iss(param);
    std::string username, hostname, servername, fullname;
    iss >> username >> hostname >> servername;
    std::getline(iss, fullname);
    if (username.empty() || hostname.empty() || servername.empty() || fullname.empty())
    {
        sendResponse(clientSocket, "Not enough parameters\n");
        return;
    }
    
    size_t i = 0;
    while (fullname[i] == ' ')
        i++;
    if (fullname[i] == ':' && fullname.length() > i + 1)
    {
        if (fullname[i + 1] == ' ')
        {
            while (fullname[i + 1] == ' ')
                i++;
            if (!fullname[i + 1])
            {
                sendResponse(clientSocket, "Invalid USER command\n");
                return;
            }
        }
        fullname = fullname.substr(i + 1);
        
    }
    else
    {
        sendResponse(clientSocket, "Invalid USER command\n");
        return;
    }

    // check if we have already the same username by another client
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second.getUsername() == username)
        {
            sendResponse(clientSocket, "Username already in use\n");
            return;
        }
    }
    
    // set the username and fullname
    _clients[clientSocket].setUsername(username);
    _clients[clientSocket].setFullname(fullname);
    
    _clients[clientSocket].setHasUsername(true);
}

void    Server::_QUIT(int clientSocket, const std::string &param)
{
    std::string paramCopy = param;
    if (!paramCopy.empty() && paramCopy[0] == ':')
        paramCopy = paramCopy.substr(1);
    std::string quitReason = paramCopy.empty() ? "Quit: " : "Quit: " + paramCopy;
    std::string notif = ":" + _clients[clientSocket].getNickname() + " QUIT :Quit: " + quitReason + "\n";
    broadcastToFriends(clientSocket, notif);
    removeClient(clientSocket);
}

void    Server::_PRIVMSG(int clientSocket, const std::string &param)
{
    if(param.empty())
    {
        sendResponse(clientSocket, "Not enough parameters\n");
        return;
    }
    std::istringstream iss(param);
    std::string target, message;
    iss >> target;
    std::getline(iss, message);
    if (target.empty() || message.empty())
    {
        sendResponse(clientSocket, "Not enough parameters\n");
        return;
    }
    size_t i = 0;
    while (message[i] == ' ')
        i++;
    if (message[i] == ':' && message.length() > i + 1)
        message = message.substr(i);
    else if (message[i] == ':' && message.length() == i + 1)
    {
        sendResponse(clientSocket, "Not enough parameters\n");
        return;
    }
    else if (message[i] == ':')
    {
        message = message.substr(i);
    }
    else
    {
        message = message.substr(i);
        size_t space = message.find(' ');
        if (space != std::string::npos)
            message = message.substr(0, space);
    }
    
    if(target[0] == '#')
    {
        if (_channels.find(target) == _channels.end())
        {
            sendResponse(clientSocket, "Channel does not exist\n");
            return;
        }
        if (_channels[target].getUsers().find(clientSocket) == _channels[target].getUsers().end())
        {
            sendResponse(clientSocket, "You are not in this channel\n");
            return;
        }

        broadcastToChannel(target, ":" + _clients[clientSocket].getNickname() + " PRIVMSG " + target + " " + message + "\n");
    }
    
    else
    {
        if (_nicknames.find(target) == _nicknames.end())
        {
            sendResponse(clientSocket, "User does not exist\n");
            return;
        }
        sendResponse(_nicknames[target], ":" + _clients[clientSocket].getNickname() + " PRIVMSG " + target + " " + message + "\n");
    }
}

void    Server::_JOIN(int clientSocket, const std::string &param)
{
    if (param.empty())
    {
        sendResponse(clientSocket, "JOIN :Not enough parameters\n");
        return;
    }

    size_t i = 0;
    while (param[i] == ' ')
        i++;

    if (i >= param.length() || (param[i] != '#' && param.length() > i + 1))
    {
        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + param.substr(i) + " :Invalid channel name\r\n");
        return;
    }

    std::string channel_name = param.substr(i);
        std::string _password; 

    size_t pos = channel_name.find(' ');
    
    if (pos != std::string::npos)
    {
        _password = channel_name.substr(pos + 1);
        if (_password.find(' ') != std::string::npos)
        {
            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + channel_name + " :Invalid channel password\n");
            return;
        }
        channel_name = channel_name.substr(0, pos);
    }
    else
    {
        _password = "";
    }
    
    // if channel doesnt exist, create it
    if (_channels.find(channel_name) == _channels.end())
    {
        _channels.insert(std::make_pair(channel_name, Channel(channel_name)));
        _channels[channel_name].addUser(clientSocket);
        _channels[channel_name].addOperator(clientSocket);
        if (!_password.empty())
            _channels[channel_name].setChannelPassword(_password);
        
    }
    // if channel exists
    else
    {   
        // if user is already in the channel
        if (_channels[channel_name].getUsers().find(clientSocket) != _channels[channel_name].getUsers().end())
        {
            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + channel_name + " :is already a member of the channel\n");
            return;
        }

        if (_channels[channel_name].isInviteOnly() && _channels[channel_name].getInvitedUsers().find(clientSocket) == _channels[channel_name].getInvitedUsers().end())
        {
            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + channel_name + " :Cannot join channel (+i)\n");
            return;
        }

        if (_channels[channel_name].getUserLimit() > 0 && _channels[channel_name].getUsers().size() >= _channels[channel_name].getUserLimit())
        {
            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + channel_name + " :Cannot join channel (+l)\n");
            return;
        }

        if (_channels[channel_name].getChannelPassword() != _password && _channels[channel_name].getInvitedUsers().find(clientSocket) == _channels[channel_name].getInvitedUsers().end())
        {
            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + channel_name + " :Cannot join channel (+k)\n");
            return;
        }
        else
        {
            _channels[channel_name].addUser(clientSocket);
        }
    }
    
    _clients[clientSocket].addChannel(channel_name);
    
    std::string joinMsg = ":" + _clients[clientSocket].getNickname() + " JOIN " + channel_name + "\r\n";
    broadcastToChannel(channel_name, joinMsg);

    std::string topic = _channels[channel_name].getTopic();
    if (!topic.empty())
        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + channel_name + " :" + topic + "\r\n");

    std::string namesList = "";
    const std::set<int>& users = _channels[channel_name].getUsers();
    for (std::set<int>::const_iterator it = users.begin(); it != users.end(); ++it)
    {
        namesList += _clients[*it].getNickname() + " ";
    }
    std::string symbol = _channels[channel_name].getChannelPassword().empty() ? "=" : "*";
    sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + symbol + " " + channel_name + " :" + namesList + "\r\n");
    sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + channel_name + " :End of /NAMES list\r\n");
}

void    Server::_KICK(int clientSocket, const std::string &param)
{
    if (param.empty())
    {
        sendResponse(clientSocket, "KICK :Not enough parameters\n");
        return;
    }

    std::istringstream iss(param);
    std::string channel, nickname, reason;
    iss >> channel >> nickname;
    std::getline(iss, reason);

    if (channel.empty() || nickname.empty())
    {
        sendResponse(clientSocket, "KICK :Not enough parameters\n");
        return;
    }

    if (_channels.find(channel) == _channels.end())
    {
        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + channel + " :Channel does not exist\n");
        return;
    }

    if (_channels[channel].getOperators().find(clientSocket) == _channels[channel].getOperators().end())
    {
        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + channel + " :You're not channel operator\n");
        return;
    }

    if (_nicknames.find(nickname) == _nicknames.end())
    {
        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + nickname + " :User does not exist\n");
        return;
    }

    if (_channels[channel].getUsers().find(_nicknames[nickname]) == _channels[channel].getUsers().end())
    {
        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + nickname + " " + channel + " :They aren't on that channel\n");
        return;
    }

    std::string notif = ":" + _clients[clientSocket].getNickname() + " KICK " + channel + " " + nickname + " ";
    if (reason.empty())
        notif += "for unknown reason\n";
    else
        notif += reason + "\n";
    broadcastToChannel(channel, notif);
    _channels[channel].removeUser(_nicknames[nickname]);
    if (_channels[channel].getOperators().find(_nicknames[nickname]) != _channels[channel].getOperators().end())
        _channels[channel].removeOperator(_nicknames[nickname]);
    if (_channels[channel].getInvitedUsers().find(_nicknames[nickname]) != _channels[channel].getInvitedUsers().end())
        _channels[channel].removeInvitedUser(_nicknames[nickname]);
}

void    Server::_TOPIC(int clientSocket, const std::string &param)
{
    bool skip = false;
    if (param.empty())
    {
        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " :Not enough parameters\n");
        return;
    }

    std::istringstream iss(param);
    std::string channel, topic;

    iss >> channel;
    std::getline(iss, topic);
    
    size_t i = 0;
    if (!topic.empty())
    {
        while (topic[i] == ' ')
            i++;
        while (topic[i] == ':')
        {
            i++;
            skip = true;
        }
        topic = topic.substr(i);
    }
    

    if (!topic.empty() && topic[0] == ':') // Remove leading ':'
    topic = topic.substr(1);
    
    if (_channels.find(channel) == _channels.end())
    {
        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + channel + " :Channel does not exist\n");
        return;
    }

    if (_channels[channel].getUsers().find(clientSocket) == _channels[channel].getUsers().end())
    {
        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + channel + " :You are not in that channel\n");
        return;
    }

    if (topic.empty())
    {
        if (_channels[channel].getOperators().find(clientSocket) == _channels[channel].getOperators().end() && (_channels[channel].hasPrivateTopic() || skip == true))
            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + channel + " :You're not channel operator\n");
        else
        {
            if (_channels[channel].getTopic().empty())
                sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + channel + " :\n");
            else
            {
                if (_channels[channel].getOperators().find(clientSocket) == _channels[channel].getOperators().end() || (_channels[channel].getOperators().find(clientSocket) != _channels[channel].getOperators().end() && skip == false))
                    sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + channel + " :" + _channels[channel].getTopic() + "\n");
                else
                {
                    _channels[channel].setTopic("");
                    std::string notif = ":" + _clients[clientSocket].getNickname() + " TOPIC " + channel + " :" + topic + "\n";
                    broadcastToChannel(channel, notif);
                }
            }
        }
    }
    
    else
    {
        if (_channels[channel].getOperators().find(clientSocket) == _channels[channel].getOperators().end())
        {
            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + channel + " :You're not channel operator\n");
            return;
        }
        else
        {
            _channels[channel].setTopic(topic);
            std::string notif = ":" + _clients[clientSocket].getNickname() + " TOPIC " + channel + " :" + topic + "\n";
            broadcastToChannel(channel, notif);
        }
    } 
}


void    Server::_INVITE(int clientSocket, const std::string &param)
{
    if (param.empty())
    {
        sendResponse(clientSocket, "INVITE :Not enough parameters\n");
        return;
    }

    std::istringstream iss(param);
    std::string nickname, channel;
    iss >> nickname >> channel;

    if (nickname.empty() || channel.empty())
    {
        sendResponse(clientSocket, "INVITE :Not enough parameters\n");
        return;
    }

    if (_nicknames.find(nickname) == _nicknames.end())
    {
        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + nickname + " :User does not exist\n");
        return;
    }

    if (_channels.find(channel) == _channels.end())
    {
        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + channel + " :No such channel\n");
        return;
    }

    if (_channels[channel].getUsers().find(clientSocket) == _channels[channel].getUsers().end())
    {
        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + channel + " :You are not in that channel\n");
        return;
    }

    if (_channels[channel].getOperators().find(clientSocket) == _channels[channel].getOperators().end() && _channels[channel].isInviteOnly())
    {
        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + channel + " :You're not channel operator\n");
        return;
    }

    _channels[channel].addInvitedUser(_nicknames[nickname]);

    sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + nickname + " " + channel + " :Invited successfully\n");
    sendResponse(_nicknames[nickname], ":" + _clients[clientSocket].getNickname() + " INVITE " + nickname + " " + channel + "\n");
}

void    Server::_MODE(int clientSocket, const std::string &param)
{
    if (param.empty())
    {
        sendResponse(clientSocket, "MODE :Not enough parameters\n");
        return;
    }
    std::istringstream iss(param);
    std::string target, mode;
    iss >> target;
    
    if (target.empty())
    {
        sendResponse(clientSocket, "MODE :Not enough parameters\n");
        return;
    }
    
    iss >> mode;
    
    if (target[0] == '#')
    {
        if (_channels.find(target) == _channels.end())
        {
            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " :No such channel\n");
            return;
        }

        if (_channels[target].getUsers().find(clientSocket) == _channels[target].getUsers().end())
        {
            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " :You are not in that channel\n");
            return;
        }

        if (mode.empty())
        {
            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " " + _channels[target].getMode() + "\n");
            return;
        }

        if (_channels[target].getOperators().find(clientSocket) == _channels[target].getOperators().end())
        {
            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " :You're not channel operator\n");
            return;
        }

        if (mode[0] != '+' && mode[0] != '-')
        {
            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " :Invalid mode\n");
            return;
        }
        for (size_t i = 1; i < mode.size(); ++i)
        {

            if (mode[i] == ' ')
                break;

            if (mode.size() == 1)
            {
                sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " :Not enough parameters\n");
                return;
            }
            
            char option = mode[i];
            
            switch (option)
            {
                case 'i':
                    if (mode[0] == '+')
                    {
                        _channels[target].setInviteOnly(true);
                        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " :Invite-only channel\n");
                    }
                    else if (mode[0] == '-')
                    {
                        _channels[target].setInviteOnly(false);
                        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " :Invite-only channel\n");
                    }
                    break;

                case 't':
                    if (mode[0] == '+')
                    {
                        _channels[target].setPrivateTopic(true);
                        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " :Topic is now private\n");
                    }
                    else if (mode[0] == '-')
                    {
                        _channels[target].setPrivateTopic(false);
                        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " :Topic is now public\n");
                    }
                    break;

                case 'k':
                    if (mode[0] == '+')
                    {
                        std::string password;
                        iss >> password;
                        if (password.empty())
                        {
                            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " :Not enough parameters\n");
                            return;
                        }
                        _channels[target].setChannelPassword(password);
                        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " :Password set\n");
                    }
                    else if (mode[0] == '-')
                    {
                        _channels[target].setChannelPassword("");
                        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " :Password removed\n");
                    }
                    break;

                case 'o':
                    if (mode[0] == '+')
                    {
                        std::string nick;
                        iss >> nick;
                        if (nick.empty())
                        {
                            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " :Not enough parameters\n");
                            return;
                        }
                        if (_channels[target].getUsers().find(_nicknames[nick]) != _channels[target].getUsers().end())
                        {
                            _channels[target].addOperator(_nicknames[nick]);
                            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " " + nick + " :Operator privileges added\n");
                            sendResponse(_nicknames[nick], _clients[clientSocket].getNickname() + " " + target + " " + nick + " :You are now a channel operator\n");
                        }
                        else
                            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target +  " " + nick + " :User does not exist\n");
                    }
                    else if (mode[0] == '-')
                    {
                        std::string nick;
                        iss >> nick;
                        if (nick.empty())
                        {
                            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " :Not enough parameters\n");
                            return;
                        }
                        if (_channels[target].getOperators().find(_nicknames[nick]) != _channels[target].getOperators().end())
                        {
                            _channels[target].removeOperator(_nicknames[nick]);
                            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " " + nick + " :Operator privileges removed\n");
                            sendResponse(_nicknames[nick], _clients[clientSocket].getNickname() + " " + target + " " + nick + " :You are no longer a channel operator\n");
                        }
                        else
                        {
                            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target +  " " + nick + " :User does not exist\n");
                        }
                    }
                    break;

                case 'l':  // User limit
                    if (mode[0] == '+')
                    {
                        int limit;
                        iss >> limit;
                        if (limit < 1)
                        {
                            sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " :Invalid limit\n");
                            return;
                        }
                        _channels[target].setUserLimit(limit);
                    }
                    else if (mode[0] == '-')
                    {
                        _channels[target].setUserLimit(0);
                    }
                    break;

                default:
                    sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " :Invalid mode\n");
                    return;
            }
        }
        broadcastToChannel(target, ":" + _clients[clientSocket].getNickname() + " " + target + " " + _channels[target].getMode() + "\n");
    }
    else
    {
        sendResponse(clientSocket, _clients[clientSocket].getNickname() + " " + target + " :Invalid channel name\n");
    }
    
}