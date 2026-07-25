#!/bin/bash
docker run --rm -v "$HOME/tmp/c42:/w" python:3.12 bash -c '
  export C_ROOT=/w/c
  sed -i "s/\r$//" /w/test_c05_07.sh /w/test_c08_10.sh
  echo "=== test c07/ex02 (error completo)"
  bash /w/test_c05_07.sh 2>&1 | grep -A1 "c07/ex02" | head -4
  TMP=$(mktemp -d)
  grep -n "ultimate_range" -A 14 /w/test_c05_07.sh | head -25
  echo "=== c08/ex01 main y error"
  grep -n "boolean" -A 16 /w/test_c08_10.sh | head -30
'
