NAME	= runner
SRC		= test.c
REPO	= ../repo
CFLAGS	= -Wall -Wextra -Werror

all: $(NAME)

$(NAME): $(SRC)
	cc $(CFLAGS) -o $(NAME) $(SRC)

test: $(NAME)
	./$(NAME) $(REPO)

clean:
	rm -f $(NAME)

re: clean all

.PHONY: all test clean re
