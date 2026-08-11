CC		= cc
FLAGS	= -Wall -Wextra -Werror
NAME	= rush-02

SRCS	= rush02.c \
		  ft_read_dict.c \
		  ft_split.c \
		  ft_strutils.c \
		  ft_write_nbr.c \
		  ft_write_utils.c \
		  auxiliar_functions.c

OBJS	= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(NAME)

# El .h como dependencia: si cambia rush02.h se recompila lo que lo usa.
%.o: %.c rush02.h
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
