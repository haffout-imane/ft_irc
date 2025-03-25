#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <string>
#include <set>

class Channel
{
    private:
        std::string _name;
        std::string _topic;
        std::string _channelPassword;
        std::set<int> _users;
        std::set<int> _operators;
        std::set<int> _invitedUsers;
        unsigned int _userLimit;
        bool _inviteOnly;
        bool _privateTopic;

    public:
        Channel();
        Channel(const std::string &name);
        Channel(Channel const &src);
        Channel &operator=(Channel const &src);
        ~Channel();

        const std::string &getName() const;
        const std::string &getTopic() const;
        const std::string &getChannelPassword() const;
        unsigned int getUserLimit() const;
        bool isInviteOnly() const;
        bool hasPrivateTopic() const;

        void addUser(int fd);
        void removeUser(int fd);
        void addInvitedUser(int fd);
        void removeInvitedUser(int fd);
        void addOperator(int fd);
        void removeOperator(int fd);
        bool isOperator(int fd) const;

        void setTopic(const std::string &topic);
        void setChannelPassword(const std::string &password);
        void setUserLimit(unsigned int limit);
        void setInviteOnly(bool status);
        void setPrivateTopic(bool status);
        
        bool isFull() const;

        const std::set<int> &getUsers() const;
        const std::set<int> &getOperators() const;
        const std::set<int> &getInvitedUsers() const;
        const std::string getMode() const;
};

#endif