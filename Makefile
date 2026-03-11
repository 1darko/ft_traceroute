NAME = ft_traceroute
CC = cc
CFLAGS = -Wall -Wextra -Werror -g 
RM = rm -f


SRCS = main.c utils.c parsing.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

run : re
	sudo ./$(NAME) 8.8.8.8

re: fclean all

.PHONY: all clean fclean re