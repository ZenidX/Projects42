#!/bin/bash
# Test maestro: ejecuta los tests de TODOS los modulos de C (c00-c13)
# encadenando los cinco scripts existentes y agrega un resumen global.
#
# Uso:
#   bash _build/test_all_c.sh              # detecta la raiz c/ del repo
#   C_ROOT=/ruta/a/c bash test_all_c.sh    # raiz personalizada
#
# Requisitos: cc (gcc), make, nm, python3, cat, tail, hexdump.
set -u

HERE=$(cd "$(dirname "$0")" && pwd)
export C_ROOT=${C_ROOT:-$(cd "$HERE/../c" && pwd)}

if [ ! -d "$C_ROOT/c00" ]; then
  echo "ERROR: no se encuentra la raiz de fuentes C en $C_ROOT" >&2
  exit 2
fi

SCRIPTS="test_c.sh test_c02_04.sh test_c05_07.sh test_c08_10.sh test_c11_13.sh"
TOTAL_PASS=0
TOTAL_FAIL=0
TOTAL_SKIP=0
FAILED_MODULES=""

echo "############################################################"
echo "##  TEST GLOBAL PISCINE C (c00-c13)  --  C_ROOT=$C_ROOT"
echo "############################################################"

for s in $SCRIPTS; do
  echo
  echo "------------------------------------------------------------"
  echo ">> $s"
  echo "------------------------------------------------------------"
  OUT=$(bash "$HERE/$s" 2>&1)
  echo "$OUT"
  P=$(printf '%s\n' "$OUT" | grep -c '^PASS: ')
  F=$(printf '%s\n' "$OUT" | grep -c '^FAIL: ')
  K=$(printf '%s\n' "$OUT" | grep -c '^SKIP: ')
  TOTAL_PASS=$((TOTAL_PASS + P))
  TOTAL_FAIL=$((TOTAL_FAIL + F))
  TOTAL_SKIP=$((TOTAL_SKIP + K))
  if [ "$F" -gt 0 ]; then
    FAILED_MODULES="$FAILED_MODULES $s($F)"
  fi
done

echo
echo "############################################################"
echo "##  RESUMEN GLOBAL: $TOTAL_PASS PASS / $TOTAL_FAIL FAIL / $TOTAL_SKIP SKIP"
if [ "$TOTAL_FAIL" -eq 0 ]; then
  echo "##  TODOS LOS TESTS DE C PASAN ✔"
else
  echo "##  Scripts con fallos:$FAILED_MODULES"
fi
echo "############################################################"
[ "$TOTAL_FAIL" -eq 0 ]
