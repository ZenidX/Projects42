#!/bin/bash
# Compila y ejecuta el runner de tests (test/test.c) de cada modulo c00-c13
# y muestra un resumen global. Uso:
#   bash c/run_tests.sh            # todos los modulos
#   bash c/run_tests.sh c06 c12    # solo los indicados
set -u

HERE=$(cd "$(dirname "$0")" && pwd)
MODS=${*:-c00 c01 c02 c03 c04 c05 c06 c07 c08 c09 c10 c11 c12 c13}
TOTAL_OK=0
TOTAL_KO=0
TOTAL_SK=0
FAILED=""

for m in $MODS; do
  d="$HERE/$m"
  if [ ! -f "$d/test/test.c" ]; then
    echo ">> $m: sin test/test.c (saltado)"
    continue
  fi
  echo "------------------------------------------------------------"
  echo ">> $m"
  echo "------------------------------------------------------------"
  if ! cc -Wall -Wextra -Werror -o "$d/test/runner" "$d/test/test.c"; then
    echo "ERROR: no compila el runner de $m"
    FAILED="$FAILED $m(runner)"
    continue
  fi
  OUT=$(cd "$d" && ./test/runner 2>&1)
  rc=$?
  echo "$OUT"
  rm -f "$d/test/runner"
  S=$(printf '%s\n' "$OUT" | tail -n 1)
  OKS=$(printf '%s' "$S" | grep -oE '[0-9]+ OK' | grep -oE '[0-9]+' || echo 0)
  KOS=$(printf '%s' "$S" | grep -oE '[0-9]+ KO' | grep -oE '[0-9]+' || echo 0)
  SKS=$(printf '%s' "$S" | grep -oE '[0-9]+ SKIP' | grep -oE '[0-9]+' || echo 0)
  TOTAL_OK=$((TOTAL_OK + OKS))
  TOTAL_KO=$((TOTAL_KO + KOS))
  TOTAL_SK=$((TOTAL_SK + SKS))
  if [ "$rc" -ne 0 ]; then
    FAILED="$FAILED $m"
  fi
  echo
done

echo "############################################################"
echo "##  TOTAL: $TOTAL_OK OK  $TOTAL_KO KO  $TOTAL_SK SKIP"
if [ -z "$FAILED" ]; then
  echo "##  TODOS LOS MODULOS PASAN ✔"
else
  echo "##  Modulos con fallos:$FAILED"
fi
echo "############################################################"
[ -z "$FAILED" ]
