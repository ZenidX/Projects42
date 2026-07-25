#!/bin/bash
# Se ejecuta EN zenidx-linux. Uso: run_tests_server.sh test_x.sh [test_y.sh ...]
# Espera ~/tmp/c42/{c,*.sh}. Compila/testea en python:3.12 y pasa norminette.
set -u
SCRIPTS="$*"
docker run --rm -v "$HOME/tmp/c42:/w" python:3.12 bash -c '
  ln -sf "$(command -v gcc)" /usr/local/bin/cc
  export C_ROOT=/w/c
  for s in '"$SCRIPTS"'; do
    echo "======== $s ========"
    sed -i "s/\r$//" "/w/$s"
    bash "/w/$s"
  done
  echo "======== NORMINETTE ========"
  pip install --quiet norminette 2>/dev/null
  cd /w/c && norminette -R CheckForbiddenSourceHeader . 2>&1 | grep -v ": OK!$" | grep -v "^Notice" | head -60
  echo "-- resumen:"
  cd /w/c && norminette -R CheckForbiddenSourceHeader . 2>&1 | grep -c ": OK!$" | xargs echo "ficheros OK:"
  cd /w/c && norminette -R CheckForbiddenSourceHeader . 2>&1 | grep -c "Error" | xargs echo "lineas con Error:"
'
