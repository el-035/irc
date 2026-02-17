CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g


SRC = srcs/main.cpp srcs/utilis/inputParse.cpp srcs/Server.cpp srcs/utilis/ServerUtils.cpp srcs/CommandHandler.cpp srcs/utilis/CommandHandlerUtils.cpp srcs/Client.cpp srcs/ChannelControl.cpp
OBJ = $(SRC:.cpp=.o)
NAME = ircserv
HEADERS = include/main.hpp include/Server.hpp include/Client.hpp include/CommandHandler.hpp 

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(OBJ) -o $(NAME)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
