#!/bin/bash
# C08 ex01/ex02 definen macros funcionales (EVEN, ABS) que el enunciado PIDE.
# Se validan con -R CheckDefine (que permite #define de macros), no con el
# CheckForbiddenSourceHeader del resto.
docker run --rm -v "$HOME/tmp/c42:/w" python:3.12 bash -c '
  pip install --quiet norminette 2>/dev/null
  cd /w/c
  echo "=== c08 ex01/ex02 con -R CheckDefine:"
  norminette -R CheckDefine c08/ex01/ft_boolean.h c08/ex02/ft_abs.h
  echo
  echo "=== resto de c08 con CheckForbiddenSourceHeader (sin ex01/ex02 .h):"
  norminette -R CheckForbiddenSourceHeader c08/ex00 c08/ex03 c08/ex04 c08/ex05 c09 c10 2>&1 | grep -v ": OK!$"
  echo "(si no hay lineas Error arriba, limpio)"
'
