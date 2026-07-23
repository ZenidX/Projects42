#!/bin/bash
# Construye los artefactos de shell00/shell01 que requieren permisos/fechas
# POSIX reales (imposibles en NTFS) y testea todas las soluciones.
set -u
ROOT=/mnt/e/WORK/Xavi/Projects42
S0=$ROOT/shells/shell00
S1=$ROOT/shells/shell01
TMP=$(mktemp -d)
PASS=0; FAIL=0
ok()   { echo "PASS: $1"; PASS=$((PASS+1)); }
bad()  { echo "FAIL: $1"; FAIL=$((FAIL+1)); }

# 0) normalizar CRLF -> LF en todo lo escrito desde Windows
find "$S0" "$S1" -type f \( -name '*.sh' -o -name 'z' -o -name 'midLS' -o -name 'clean' -o -name 'ft_magic' -o -name 'b' \) -exec sed -i 's/\r$//' {} +

### ------- SHELL 00 -------

# ex00: z
[ "$(cat "$S0/ex00/z")" = "Z" ] && ok "s00/ex00 cat z muestra Z" || bad "s00/ex00"

# ex01: testShell00.tar  (-r--r-xr-x, 40 bytes, Jun 1 23:42)
mkdir -p "$TMP/ex01" && cd "$TMP/ex01"
printf '%040d' 0 > testShell00
chmod 455 testShell00
touch -t 06012342 testShell00
tar -cf testShell00.tar testShell00
cp testShell00.tar "$S0/ex01/"
perm=$(tar -tvf testShell00.tar | awk '{print $1, $3}')
[ "$perm" = "-r--r-xr-x 40" ] && ok "s00/ex01 tar: $perm Jun 1 23:42" || bad "s00/ex01 tar dice: $perm"

# ex02: exo2.tar
mkdir -p "$TMP/ex02" && cd "$TMP/ex02"
mkdir test0 test2
printf 'abc\n' > test1        # 4 bytes
printf '1'    > test3         # 1 byte
printf '2\n'  > test4         # 2 bytes
ln test3 test5                # hard link (2 enlaces)
ln -s test0 test6             # symlink
chmod 715 test0; chmod 714 test1; chmod 504 test2; chmod 404 test3; chmod 641 test4
touch -t 06012047 test0; touch -t 06012146 test1; touch -t 06012245 test2
touch -t 06012344 test3 test5; touch -t 06012343 test4; touch -h -t 06012220 test6
tar -cf exo2.tar *
cp exo2.tar "$S0/ex02/"
echo "--- contenido exo2.tar:"; tar -tvf exo2.tar
n=$(tar -tvf exo2.tar | grep -c -E '^(drwx--xr-x .* test0|-rwx--xr-- .* test1|dr-x---r-- .* test2|-r-----r-- .* test3|-rw-r----x .* test4|-r-----r-- .* test5|lrwxrwxrwx .* test6 -> test0)')
[ "$n" = 7 ] && ok "s00/ex02 los 7 permisos correctos" || bad "s00/ex02 solo $n/7 correctos"

# ex03: clave ed25519 nueva
mkdir -p "$TMP/ex03"
ssh-keygen -t ed25519 -N "" -C "xalara@42barcelona" -f "$TMP/ex03/id_ed25519" -q
mkdir -p "$S0/ex03"
cp "$TMP/ex03/id_ed25519.pub" "$S0/ex03/id_ed25519_pub"
mkdir -p "$ROOT/_keys" && cp "$TMP/ex03/id_ed25519" "$ROOT/_keys/id_ed25519_shell00" && chmod 600 "$ROOT/_keys/id_ed25519_shell00"
grep -q '^ssh-ed25519 ' "$S0/ex03/id_ed25519_pub" && ok "s00/ex03 clave publica ed25519 generada" || bad "s00/ex03"

# ex04: midLS
mkdir -p "$TMP/ex04/dirA" && cd "$TMP/ex04"
touch fich1; sleep 0.1; touch .oculto; touch dirA/x
out=$(sh -c "$(cat "$S0/ex04/midLS")")
echo "--- midLS: $out"
case "$out" in *"dirA/"*) case "$out" in *".oculto"*) bad "s00/ex04 muestra ocultos";; *) ok "s00/ex04 midLS (dirs con /, sin ocultos, formato coma)";; esac;; *) bad "s00/ex04 sin barra en dirs: $out";; esac

# ex05: git_commit.sh — repo de prueba con 6 commits
mkdir -p "$TMP/ex05" && cd "$TMP/ex05"
git init -q .; git config user.email t@t; git config user.name t
for i in 1 2 3 4 5 6; do echo $i > f; git add f; git commit -qm "c$i"; done
out=$(sh "$S0/ex05/git_commit.sh")
[ "$(echo "$out" | wc -l)" = 5 ] && [ "$(echo "$out" | head -1)" = "$(git rev-parse HEAD)" ] && ok "s00/ex05 5 ultimos commits" || bad "s00/ex05: $out"

# ex06: git_ignore.sh
echo "*.tmp" > .gitignore; git add .gitignore; git commit -qm gi; touch a.tmp b.tmp
out=$(sh "$S0/ex06/git_ignore.sh" | sort | tr '\n' ' ')
[ "$out" = "a.tmp b.tmp " ] && ok "s00/ex06 lista ignorados" || bad "s00/ex06: $out"

# ex07: b = a parcheado con sw.diff, y diff a b reproduce sw.diff
cd "$S0/resources"
cp a "$TMP/a"; cp sw.diff "$TMP/sw.diff"
cd "$TMP"; cp a b; patch -s b < sw.diff
mkdir -p "$S0/ex07"; cp b "$S0/ex07/b"
diff a b > regen.diff; diff -q regen.diff sw.diff >/dev/null && ok "s00/ex07 diff a b == sw.diff" || bad "s00/ex07 el diff no coincide"

# ex08: clean
mkdir -p "$TMP/ex08/sub" && cd "$TMP/ex08"
touch "borrame~" "sub/otro~" "#tmp#" "sub/#x#" "normal.txt" "no#tocar"
sh -c "$(cat "$S0/ex08/clean")" > /dev/null
restantes=$(find . -type f | sort | tr '\n' ' ')
[ "$restantes" = "./no#tocar ./normal.txt " ] && ok "s00/ex08 borra ~ y #...# y respeta el resto" || bad "s00/ex08 quedan: $restantes"

# ex09: ft_magic
cd "$TMP"
printf '%042d42' 0 > f42   # "42" en el byte 42 (offset 42)
printf '%042d13' 0 > f13
r1=$(file -m "$S0/ex09/ft_magic" f42); r2=$(file -m "$S0/ex09/ft_magic" f13)
case "$r1" in *"42 file"*) case "$r2" in *"42 file"*) bad "s00/ex09 falso positivo";; *) ok "s00/ex09 file detecta '42 file'";; esac;; *) bad "s00/ex09: $r1";; esac

### ------- SHELL 01 -------

# ex01: print_groups.sh
export FT_USER=daemon
out=$(sh "$S1/ex01/print_groups.sh")
[ "$out" = "daemon" ] || [ "$out" = "daemon,bin" ] && ok "s01/ex01 grupos de daemon: $out" || bad "s01/ex01: $out"

# ex02: find_sh.sh
mkdir -p "$TMP/s1ex02/sub" && cd "$TMP/s1ex02"
touch file1.sh sub/file2.sh normal.txt
out=$(sh "$S1/ex02/find_sh.sh" | sort | tr '\n' ' ')
[ "$out" = "file1 file2 " ] && ok "s01/ex02 nombres sin .sh ni ruta" || bad "s01/ex02: $out"

# ex03: count_files.sh
mkdir -p "$TMP/s1ex03/d1/d2" && cd "$TMP/s1ex03"
touch f1 d1/f2 d1/d2/f3   # 3 ficheros + 3 dirs (., d1, d2) = 6
out=$(sh "$S1/ex03/count_files.sh")
[ "$out" = "6" ] && ok "s01/ex03 cuenta 6 (3f+3d incl .)" || bad "s01/ex03: $out"

# ex04: MAC.sh
out=$(sh "$S1/ex04/MAC.sh")
echo "$out" | grep -qE '^([0-9a-f]{2}:){5}[0-9a-f]{2}$' && ok "s01/ex04 MACs: $(echo $out | head -1)" || bad "s01/ex04 (puede no haber ifconfig): $out"

# ex05: archivo MaRViN — crear y empaquetar
mkdir -p "$TMP/s1ex05" && cd "$TMP/s1ex05"
sed 's/\r$//' "$S1/ex05/create_marvin.sh" > cm.sh
sh cm.sh
fname=$(ls | head -1)
size=$(stat -c%s "$fname")
[ "$fname" = '"\?$*'\''MaRViN'\''*$?\"' ] && [ "$size" = 2 ] && ok "s01/ex05 archivo MaRViN creado (2 bytes)" || bad "s01/ex05 nombre=$fname size=$size"
tar -cf "$S1/ex05/ex05.tar" "$fname"

# ex06: skip.sh
cd "$TMP/s1ex05"  # cualquier dir con ficheros
out_all=$(ls -l | wc -l); out=$(sh "$S1/ex06/skip.sh" | wc -l)
exp=$(( (out_all + 1) / 2 ))
[ "$out" = "$exp" ] && ok "s01/ex06 1 de cada 2 lineas ($out de $out_all)" || bad "s01/ex06: $out vs $exp"

# ex07: r_dwssap.sh
export FT_LINE1=7 FT_LINE2=15
out=$(sh "$S1/ex07/r_dwssap.sh")
echo "--- r_dwssap: $out"
n=$(echo "$out" | tr -cd ',' | wc -c)
case "$out" in *.) [ "$n" = 8 ] && ok "s01/ex07 9 nombres, coma-espacio, termina en punto" || bad "s01/ex07 comas=$n";; *) bad "s01/ex07 no termina en punto";; esac

# ex08: add_chelou.sh — verificacion aritmetica propia:
# n1 = "?!  (base5: 2 3 4) = 2*25+3*5+4 = 69 ; n2 = rc (1 4) = 9 ; 69+9=78 = 6*13+0 -> "lg"
export FT_NBR1='"?!' FT_NBR2=rc
out=$(sh "$S1/ex08/add_chelou.sh")
[ "$out" = "lg" ] && ok "s01/ex08 aritmetica en bases raras ok (78 -> lg)" || bad "s01/ex08: '$out' (esperaba lg)"

echo
echo "==== RESULTADO: $PASS PASS / $FAIL FAIL ===="
rm -rf "$TMP"
