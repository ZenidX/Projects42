CC		= cc
FLAGS	= -Wall -Wextra -Werror
NAME	= bsq 

SRCS	= bsq.c \
		  ft_read_map.c \
		  ft_auxiliar.c
		  ft_split.c \
		  ft_str_utils.c \
		  ft_write_nbr.c \
		  ft_write_utils.c \

OBJS	= $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(NAME)

# El .h como dependencia: si cambia bsq.h se recompila lo que lo usa.
%.o: %.c bsq.h
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
