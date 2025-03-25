#include "Client.hpp"

Client::Client() : _fd(-1), _isAuthenticated(false), _isOperator(false), bot(false)
{
}

Client::Client(int fd) : _fd(fd), _isAuthenticated(false), _isOperator(false)
{
}

Client::Client(Client const &src)
{
    *this = src;
}

Client &Client::operator=(Client const &src)
{
    _fd = src._fd;
    _nickname = src._nickname;
    _username = src._username;
    _fullname = src._fullname;
    _channels = src._channels;
    _isAuthenticated = src._isAuthenticated;
    _isOperator = src._isOperator;
    _buffer = src._buffer;
    return *this;
}

Client::~Client()
{
}

int Client::getFd() const
{
    return _fd;
}

const std::string &Client::getNickname() const
{
    return _nickname;
}

const std::string &Client::getUsername() const
{
    return _username;
}

const std::string &Client::getFullname() const
{
    return _fullname;
}

bool Client::isAuthenticated() const
{
    return _isAuthenticated;
}   

bool Client::hasNickname() const
{
    return _hasNickname;
}

bool Client::hasUsername() const
{
    return _hasUsername;
}

bool Client::knowsPassword() const
{
    return _knowsPassword;
}

bool Client::isOperator() const
{
    return _isOperator;
}

const std::set<std::string> &Client::getChannels() const
{
    return _channels;
}

const std::string &Client::getBuffer() const
{
    return _buffer;
}

void Client::setNickname(const std::string &nickname)
{
    _nickname = nickname;
}

void Client::setUsername(const std::string &username)
{
    _username = username;
}

void Client::setFullname(const std::string &fullname)
{
    _fullname = fullname;
}

void Client::setBuffer(const std::string &buffer)
{
    _buffer = buffer;
}

void Client::setAuthenticated(bool status)
{
    _isAuthenticated = status;
}

void Client::setOperator(bool status)
{
    _isOperator = status;
}

void Client::setHasNickname(bool status)
{
    _hasNickname = status;
}

void Client::setHasUsername(bool status)
{
    _hasUsername = status;
}

void Client::setKnowsPassword(bool status)
{
    _knowsPassword = status;
}

void Client::appendBuffer(const std::string &data)
{
    _buffer.append(data);
}

void Client::clearBuffer()
{
    _buffer.clear();
}

void Client::addChannel(const std::string &channel)
{
    _channels.insert(channel);
}

void Client::removeChannel(const std::string &channel)
{
    _channels.erase(channel);
}

bool Client::isInChannel(const std::string &channel) const
{
    if (_channels.find(channel) == _channels.end())
        return false;
    return true;
}

bool Client::usingBot() const
{
    return bot;
}

void Client::setBot(bool status)
{
    bot = status;
}