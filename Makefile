NAME = ircserv

SRCS = srcs/
INCLUDE = includes/

CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -g

FILES = main.cpp Server.cpp MessagesParse.cpp User.cpp Channel.cpp utils.cpp \
		Botard.cpp
OBJECTS = $(addprefix $(SRCS), $(FILES:.cpp=.o))

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INCLUDE)

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ -I $(INCLUDE)

bonus: all

clean:
	rm -rf $(OBJECTS)


fclean: clean
	rm -rf $(NAME)

re: fclean all
