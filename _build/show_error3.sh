#!/bin/bash
docker run --rm -v "$HOME/tmp/c42:/w" python:3.12 bash -c '
cat > /tmp/m01.c <<EOF
#include <unistd.h>
#include "ft_boolean.h"
void ft_putstr(char *str){while (*str) write(1, str++, 1);}
t_bool ft_is_even(int nbr){return ((EVEN(nbr)) ? TRUE : FALSE);}
int main(int argc, char **argv){(void)argv;
if (ft_is_even(argc - 1) == TRUE) ft_putstr(EVEN_MSG);
else ft_putstr(ODD_MSG); return (SUCCESS);}
EOF
gcc -Wall -Wextra -Werror -I /w/c/c08/ex01 /tmp/m01.c -o /tmp/b01 2>&1 | head -12
echo "=== ft_boolean.h:"
cat /w/c/c08/ex01/ft_boolean.h | tail -20
'
