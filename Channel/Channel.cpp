#include "Channel.hpp"

Channel::Channel() : _userLimit(0), _inviteOnly(false), _privateTopic(false)
{
}

Channel::Channel(const std::string &name) : _name(name), _userLimit(0), _inviteOnly(false), _privateTopic(false)
{
}

Channel::Channel(Channel const &src)
{
    *this = src;
}

Channel &Channel::operator=(Channel const &src)
{
    _name = src._name;
    _topic = src._topic;
    _channelPassword = src._channelPassword;
    _users = src._users;
    _operators = src._operators;
    _userLimit = src._userLimit;
    _inviteOnly = src._inviteOnly;
    _privateTopic = src._privateTopic;
    return *this;
}

Channel::~Channel()
{
}

const std::string &Channel::getName() const
{
    return _name;
}

const std::string &Channel::getTopic() const
{
    return _topic;
}

const std::string &Channel::getChannelPassword() const
{
    return _channelPassword;
}

unsigned int Channel::getUserLimit() const
{
    return _userLimit;
}

bool Channel::isInviteOnly() const
{
    return _inviteOnly;
}

bool Channel::hasPrivateTopic() const
{
    return _privateTopic;
}

void Channel::addUser(int fd)
{
    _users.insert(fd);
}

void Channel::removeUser(int fd)
{
    _users.erase(fd);
}

void Channel::addInvitedUser(int fd)
{
    _invitedUsers.insert(fd);
}

void Channel::removeInvitedUser(int fd)
{
    _invitedUsers.erase(fd);
}

void Channel::addOperator(int fd)
{
    _operators.insert(fd);
}

void Channel::removeOperator(int fd)
{
    _operators.erase(fd);
}

bool Channel::isOperator(int fd) const
{
    if (_operators.find(fd) != _operators.end())
        return true;
    return false;
}

void Channel::setTopic(const std::string &topic)
{
    _topic = topic;
}

void Channel::setChannelPassword(const std::string &password)
{
    _channelPassword = password;
}

void Channel::setUserLimit(unsigned int limit)
{
    _userLimit = limit;
}

void Channel::setInviteOnly(bool status)
{
    _inviteOnly = status;
}

void Channel::setPrivateTopic(bool status)
{
    _privateTopic = status;
}

bool Channel::isFull() const
{
    if (_userLimit > 0 && _users.size() >= _userLimit)
        return true;
    return false;
}

const std::set<int> &Channel::getUsers() const
{
    return _users;
}

const std::set<int> &Channel::getOperators() const
{
    return _operators;
}

const std::set<int> &Channel::getInvitedUsers() const
{
    return _invitedUsers;
}

const std::string Channel::getMode() const
{
    std::string result;

    result += (_channelPassword.empty() ? "" : "k");
    result += (_inviteOnly ? "i" : "");
    result += (_privateTopic ? "t" : "");
    result += (_userLimit == 0 ? "" : "l");
    if (!result.empty())
        result = "+" + result;
    else
        result = "No modes set";
    return (result);
}