#!/bin/bash
set -u
docker run --rm -v "$HOME/tmp/c42:/w" python:3.12 bash -c '
  ln -sf "$(command -v gcc)" /usr/local/bin/cc
  cat > /tmp/main.c <<EOF
#include <stdio.h>

void ft_rev_int_tab(int *t, int s);

int main(void)
{
    int t[5] = {1, 2, 3, 4, 5};
    int u[4] = {9, 8, 7, 6};
    int i;

    ft_rev_int_tab(t, 5);
    ft_rev_int_tab(u, 4);
    for (i = 0; i < 5; i++)
        printf("%d", t[i]);
    printf(" ");
    for (i = 0; i < 4; i++)
        printf("%d", u[i]);
    return (0);
}
EOF
  cc -Wall -Wextra -Werror /w/c/c01/ex07/ft_rev_int_tab.c /tmp/main.c -o /tmp/prog && out=$(/tmp/prog)
  echo "salida: [$out]"
  if [ "$out" = "54321 6789" ]; then echo "PASS: c01/ex07 rev_int_tab"; else echo "FAIL"; fi
'
