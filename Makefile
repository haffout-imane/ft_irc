NAME = ircserv

SRCS = main.cpp ./Server/Server.cpp ./Client/Client.cpp ./Channel/Channel.cpp ./Commands/Commands.cpp ./Bot/Bot.cpp ./Bot/ChannelDisplay.cpp ./Bot/DadJokes.cpp ./Bot/Roast.cpp

OBJS = $(SRCS:.cpp=.o)

CFLAGS = -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME): $(OBJS) ./Server/Server.hpp ./Client/Client.hpp ./Channel/Channel.hpp
	c++ $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp ./Server/Server.hpp ./Client/Client.hpp ./Channel/Channel.hpp
	c++ $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re