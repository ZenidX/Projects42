#!/bin/bash
docker run --rm -v "$HOME/tmp/c42:/w" python:3.12 bash -c '
  for f in c12/ex07/ft_list_at.c c12/ex08/ft_list_reverse.c c12/ex11/ft_list_find.c c13/ex05/btree_search_item.c; do
    echo "=== $f"
    gcc -Wall -Wextra -Werror -c "/w/c/$f" -o /dev/null 2>&1 | head -8
  done
  echo "=== c11 fallos previos (recompilar todo c11)"
  for f in /w/c/c11/ex*/*.c; do
    gcc -Wall -Wextra -Werror -fsyntax-only "$f" 2>&1 | head -4
  done
'
