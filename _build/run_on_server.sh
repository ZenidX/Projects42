#!/bin/bash
# Se ejecuta EN zenidx-linux: tests de C en contenedor python:3.12 (trae gcc)
# y norminette en el mismo contenedor via pip.
set -u
docker run --rm -v "$HOME/tmp/c42:/w" python:3.12 bash -c '
  ln -sf "$(command -v gcc)" /usr/local/bin/cc
  sed -i "s|/mnt/e/WORK/Xavi/Projects42/c|/w/c|" /w/test_c.sh
  sed -i "s/\r$//" /w/test_c.sh
  bash /w/test_c.sh
  echo
  echo "==== NORMINETTE ===="
  pip install --quiet norminette 2>/dev/null
  cd /w/c && norminette -R CheckForbiddenSourceHeader c00 c01 | grep -v "^Notice" | tail -40
'
