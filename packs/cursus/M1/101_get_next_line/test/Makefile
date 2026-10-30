NAME	= runner
SRC		= test.c
REPO	= ../repo
CFLAGS	= -Wall -Wextra -Werror

# La parte obligatoria ya cumple el bonus: los _bonus son una copia de ella
# en la que solo cambian el nombre en la cabecera de 42 (comiendose 6
# espacios para no descuadrarla), el include y el guard del header.
SYNC	= s/get_next_line_utils\.c      /get_next_line_utils_bonus.c/; \
		  s/get_next_line\.c      /get_next_line_bonus.c/; \
		  s/get_next_line\.h      /get_next_line_bonus.h/; \
		  s/"get_next_line\.h"/"get_next_line_bonus.h"/; \
		  s/GET_NEXT_LINE_H/GET_NEXT_LINE_BONUS_H/g

all: bonus $(NAME)

$(NAME): $(SRC)
	cc $(CFLAGS) -o $(NAME) $(SRC)

bonus:
	@for f in get_next_line.c get_next_line_utils.c get_next_line.h; do \
		[ -f $(REPO)/$$f ] || continue; \
		b=$$(echo $$f | sed 's/\.\([ch]\)$$/_bonus.\1/'); \
		sed '$(SYNC)' $(REPO)/$$f > $(REPO)/$$b; \
		echo "$$f -> $$b"; \
	done

test: all
	./$(NAME) $(REPO)

clean:
	rm -f $(NAME)

re: clean all

.PHONY: all bonus test clean re
