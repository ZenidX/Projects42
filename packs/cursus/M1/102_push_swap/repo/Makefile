# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/10/24 15:00:00 by xalara            #+#    #+#              #
#    Updated: 2026/10/24 15:00:00 by xalara           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= push_swap

CC		= cc
CFLAGS	= -Wall -Wextra -Werror

LIBFT_DIR	= libft
LIBFT	= $(LIBFT_DIR)/libft.a

SRCS	= push_swap.c parse.c index.c bench.c \
		  swap.c push.c rotate.c rotate_reverse.c \
		  sort_simple.c sort_medium.c sort_complex.c sort_adaptive.c

OBJS	= $(SRCS:.c=.o)

all: libft $(NAME)

libft:
	$(MAKE) -C $(LIBFT_DIR)

$(LIBFT): libft

$(NAME): $(OBJS) $(LIBFT)
	$(CC) $(CFLAGS) $(OBJS) $(LIBFT) -o $(NAME)

%.o: %.c push_swap.h $(LIBFT_DIR)/libft.h
	$(CC) $(CFLAGS) -I$(LIBFT_DIR) -c $< -o $@

clean:
	$(MAKE) -C $(LIBFT_DIR) clean
	rm -f $(OBJS)

fclean: clean
	$(MAKE) -C $(LIBFT_DIR) fclean
	rm -f $(NAME)

re: fclean all

.PHONY: all libft clean fclean re
