#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <set>

class Client
{
    private:
        int _fd;
        std::string _nickname;
        std::string _username;
        std::string _fullname;  
        std::set<std::string> _channels;
        bool _hasUsername;
        bool _hasNickname;
        bool _knowsPassword;
        bool _isAuthenticated;
        bool _isOperator;
        bool bot;
        std::string _buffer;

    public:
        Client();
        Client(int fd);
        ~Client();
        Client(Client const &src);
        Client &operator=(Client const &src);

        int getFd() const;
        const std::string &getNickname() const;
        const std::string &getUsername() const;
        const std::string &getFullname() const;
        bool isAuthenticated() const;
        bool isOperator() const;
        bool hasUsername() const;
        bool hasNickname() const;
        bool knowsPassword() const;
        bool usingBot() const;
        void setBot(bool status);
        
        const std::set<std::string> &getChannels() const;
        const std::string &getBuffer() const;

        void setNickname(const std::string &nickname);
        void setUsername(const std::string &username);
        void setFullname(const std::string &fullname);
        void setBuffer(const std::string &buffer);
        void setAuthenticated(bool status);
        void setOperator(bool status);
        void setHasUsername(bool status);
        void setHasNickname(bool status);
        void setKnowsPassword(bool status);

        void appendBuffer(const std::string &data);
        void clearBuffer();

        void addChannel(const std::string &channel);
        void removeChannel(const std::string &channel);
        bool isInChannel(const std::string &channel) const;
        
};

#endif