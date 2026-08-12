CC		= cc
FLAGS	= -Wall -Wextra -Werror
NAME	= bsq

SRCS	= bsq.c \
		  ft_read_file_map.c \
		  ft_parse_map.c \
		  ft_parse_header.c \
		  ft_bsq_isolver.c \
		  ft_print_map.c \
		  ft_auxiliar.c \
		  ft_split.c \
		  ft_str_utils.c

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
