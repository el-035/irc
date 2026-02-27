CXX = c++
# remove -g later
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g

# If you want to use the Sanitizer, uncomment the next two lines:
# CXXFLAGS += -fsanitize=address
# LDFLAGS += -fsanitize=address

SRC = srcs/main.cpp srcs/utilis/inputParse.cpp srcs/Server.cpp \
		srcs/utilis/ServerUtils.cpp srcs/CommandHandler.cpp \
		srcs/utilis/CommandHandlerUtils.cpp srcs/Client.cpp \
		srcs/CommandHandlerChannelControl.cpp srcs/registration.cpp

OBJ = $(SRC:.cpp=.o)
NAME = ircserv
HEADERS = include/main.hpp include/Server.hpp include/Client.hpp include/CommandHandler.hpp

PORT = 2000
PASS = 2

all: $(NAME)

# Added $(LDFLAGS) here so Sanitizer works
$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

# Only run this if -fsanitize=address is COMMENTED OUT above
val: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes ./$(NAME) $(PORT) $(PASS)

run: $(NAME)
	./$(NAME) $(PORT) $(PASS)

.PHONY: all clean fclean re val run
