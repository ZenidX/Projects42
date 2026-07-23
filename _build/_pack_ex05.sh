#!/bin/bash
set -eu
S1=/mnt/e/WORK/Xavi/Projects42/shells/shell01
TMP=$(mktemp -d)
cd "$TMP"
sed 's/\r$//' "$S1/ex05/create_marvin.sh" > cm.sh
sh cm.sh
rm cm.sh
fname=$(find . -maxdepth 1 -type f -printf '%f\n')
echo "empaquetando: [$fname]"
printf '%s\0' "$fname" | tar --null --no-unquote -cf "$S1/ex05/ex05.tar" -T -
echo "--- contenido del tar:"
tar -tvf "$S1/ex05/ex05.tar"
cd /
rm -rf "$TMP"
