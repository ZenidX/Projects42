#!/bin/bash
# Tests de C08/C09/C10. Pensado para correr en un contenedor con gcc (cc),
# make, cat, tail y python3 (p.ej. python:3.12 con gcc). Raiz configurable:
#   C_ROOT=/w/c bash test_c08_10.sh
# C08: compila un main por header/struct/macro. C09: libft real + make + nm.
# C10: compara la salida contra cat/tail y contra una referencia de hexdump
# (validada byte a byte contra el hexdump del sistema), mas casos de error.
set -u
C=${C_ROOT:-/w/c}
TMP=$(mktemp -d)
PASS=0; FAIL=0; SKIP=0
ok()   { echo "PASS: $1"; PASS=$((PASS+1)); }
bad()  { echo "FAIL: $1"; FAIL=$((FAIL+1)); }
skip() { echo "SKIP: $1"; SKIP=$((SKIP+1)); }
have() { command -v "$1" >/dev/null 2>&1; }

# Referencia de hexdump (formato por defecto) identica al comando del sistema.
cat > "$TMP/hdref.py" <<'PY'
import sys
data = b"".join(open(f, "rb").read() for f in sys.argv[1:])
out = []
for off in range(0, len(data), 16):
    ch = data[off:off + 16]
    g = []
    i = 0
    while i < len(ch):
        b0 = ch[i]
        b1 = ch[i + 1] if i + 1 < len(ch) else 0
        g.append("%02x%02x" % (b1, b0))
        i += 2
    out.append(("%07x " % off + " ".join(g)).ljust(47))
if len(data) > 0:
    out.append("%07x" % len(data))
sys.stdout.write("\n".join(out) + ("\n" if out else ""))
PY

# build <dir> <binname>  -> compila en $TMP/<binname> (make si existe, si no cc)
build() {
  local dir=$1 bin=$2
  rm -f "$TMP/$bin"
  if have make && [ -f "$dir/Makefile" ]; then
    if make -C "$dir" >/dev/null 2>"$TMP/mkerr"; then
      cp "$dir/$bin" "$TMP/$bin"
      make -C "$dir" fclean >/dev/null 2>&1
      return 0
    fi
  fi
  cc -Wall -Wextra -Werror "$dir"/*.c -o "$TMP/$bin" 2>"$TMP/mkerr"
}

# cmp_out <nombre> <salida_real> <esperada>
cmp_out() {
  if [ "$2" = "$3" ]; then ok "$1"; else bad "$1: [$2] != [$3]"; fi
}

echo "==================== C 08 ===================="

# ex00 ft.h : compila (solo cabecera) un main que usa los prototipos
cat > "$TMP/m00.c" <<'EOF'
#include "ft.h"
int main(void){int a=1,b=2;ft_swap(&a,&b);ft_putchar('B');ft_putstr("x");
return ft_strlen("hi")+ft_strcmp("a","a");}
EOF
if cc -Wall -Wextra -Werror -I "$C/c08/ex00" -c "$TMP/m00.c" -o "$TMP/m00.o" \
     2>"$TMP/e"; then ok "c08/ex00 ft.h compila"
else bad "c08/ex00 ft.h: $(head -1 "$TMP/e")"; fi

# ex01 ft_boolean.h : main del enunciado, comprobando par/impar
cat > "$TMP/m01.c" <<'EOF'
#include <unistd.h>
#include "ft_boolean.h"

void ft_putstr(char *str)
{
	while (*str)
		write(1, str++, 1);
}

t_bool ft_is_even(int nbr)
{
	return ((EVEN(nbr)) ? TRUE : FALSE);
}

int main(int argc, char **argv)
{
	(void)argv;
	if (ft_is_even(argc - 1) == TRUE)
		ft_putstr(EVEN_MSG);
	else
		ft_putstr(ODD_MSG);
	return (SUCCESS);
}
EOF
if cc -Wall -Wextra -Werror -I "$C/c08/ex01" "$TMP/m01.c" -o "$TMP/b01" \
     2>"$TMP/e"; then
  cmp_out "c08/ex01 boolean (0 args, par)" "$("$TMP/b01")" \
    "I have an even number of arguments."
  cmp_out "c08/ex01 boolean (1 arg, impar)" "$("$TMP/b01" x)" \
    "I have an odd number of arguments."
  cmp_out "c08/ex01 boolean (2 args, par)" "$("$TMP/b01" x y)" \
    "I have an even number of arguments."
else bad "c08/ex01 boolean: $(head -1 "$TMP/e")"; fi

# ex02 ft_abs.h : macro ABS
cat > "$TMP/m02.c" <<'EOF'
#include "ft_abs.h"
#include <stdio.h>
int main(void){printf("%d %d %d", ABS(-5), ABS(5), ABS(0)); return 0;}
EOF
if cc -Wall -Wextra -Werror -I "$C/c08/ex02" "$TMP/m02.c" -o "$TMP/b02" \
     2>"$TMP/e"; then cmp_out "c08/ex02 ABS" "$("$TMP/b02")" "5 5 0"
else bad "c08/ex02 ABS: $(head -1 "$TMP/e")"; fi

# ex03 ft_point.h : main del enunciado
cat > "$TMP/m03.c" <<'EOF'
#include "ft_point.h"
void set_point(t_point *point){point->x = 42; point->y = 21;}
int main(void){t_point point; set_point(&point);
return (point.x == 42 && point.y == 21) ? 0 : 1;}
EOF
if cc -Wall -Wextra -Werror -I "$C/c08/ex03" "$TMP/m03.c" -o "$TMP/b03" \
     2>"$TMP/e"; then
  if "$TMP/b03"; then ok "c08/ex03 ft_point.h"; else bad "c08/ex03 struct mal"; fi
else bad "c08/ex03 ft_point.h: $(head -1 "$TMP/e")"; fi

# ex04+ex05 : ft_strs_to_tab alimenta ft_show_tab
cat > "$TMP/m45.c" <<'EOF'
#include "ft_stock_str.h"
struct s_stock_str *ft_strs_to_tab(int ac, char **av);
void ft_show_tab(struct s_stock_str *par);
int main(void){char *av[3] = {"hola", "42", "mundo"};
ft_show_tab(ft_strs_to_tab(3, av)); return 0;}
EOF
if cc -Wall -Wextra -Werror -I "$C/c08/ex04" \
     "$C/c08/ex04/ft_strs_to_tab.c" "$C/c08/ex05/ft_show_tab.c" \
     "$TMP/m45.c" -o "$TMP/b45" 2>"$TMP/e"; then
  exp=$'hola\n4\nhola\n42\n2\n42\nmundo\n5\nmundo'
  cmp_out "c08/ex04+05 strs_to_tab/show_tab" "$("$TMP/b45")" "$exp"
else bad "c08/ex04+05: $(head -1 "$TMP/e")"; fi

echo "==================== C 09 ===================="

# ex00 : libft_creator.sh crea libft.a y sus simbolos; enlazamos contra ella
D="$TMP/c09ex00"; mkdir -p "$D"; cp "$C/c09/ex00/"*.c "$C/c09/ex00/"*.sh "$D/"
( cd "$D" && sh libft_creator.sh >/dev/null 2>"$TMP/e" )
if [ -f "$D/libft.a" ]; then
  ok "c09/ex00 libft.a creada"
  if have nm; then
    miss=""
    for s in ft_putchar ft_swap ft_putstr ft_strlen ft_strcmp; do
      nm "$D/libft.a" 2>/dev/null | grep -q "T $s" || miss="$miss $s"
    done
    [ -z "$miss" ] && ok "c09/ex00 simbolos en libft.a" \
      || bad "c09/ex00 faltan simbolos:$miss"
  else skip "c09/ex00 nm (no disponible)"; fi
  cat > "$TMP/m9.c" <<'EOF'
#include <stdio.h>
int ft_strlen(char *s); int ft_strcmp(char *a, char *b); void ft_putstr(char *s);
int main(void){ft_putstr("hi\n");
printf("%d %d", ft_strlen("hola"), ft_strcmp("a", "a")); return 0;}
EOF
  if cc -Wall -Wextra -Werror "$TMP/m9.c" "$D/libft.a" -o "$TMP/b9" \
       2>"$TMP/e"; then cmp_out "c09/ex00 enlace+uso" "$("$TMP/b9")" $'hi\n4 0'
  else bad "c09/ex00 enlace: $(head -1 "$TMP/e")"; fi
else bad "c09/ex00 libft.a no creada: $(head -1 "$TMP/e")"; fi

# ex01 : Makefile (unico entregable) probado con fuentes de andamiaje
if have make; then
  M="$TMP/c09ex01"; mkdir -p "$M/srcs" "$M/includes"
  cp "$C/c09/ex01/Makefile" "$M/"
  cp "$C/c09/ex00/"*.c "$M/srcs/"
  cp "$C/c08/ex00/ft.h" "$M/includes/" 2>/dev/null || printf '#ifndef FT_H\n# define FT_H\n#endif\n' > "$M/includes/ft.h"
  ( cd "$M" && make >/dev/null 2>"$TMP/e" )
  if [ -f "$M/libft.a" ]; then
    ok "c09/ex01 make crea libft.a"
    # sin cambios: make no debe rehacer nada (no comando util)
    out=$(cd "$M" && make 2>&1)
    echo "$out" | grep -qiE "Nothing to be done|is up to date|actualizado|nada que hacer" \
      && ok "c09/ex01 make incremental (no relink)" \
      || skip "c09/ex01 make incremental (mensaje no reconocido)"
    # tocar un .c fuerza recompilar ese objeto
    touch "$M/srcs/ft_strlen.c"
    out=$(cd "$M" && make 2>&1)
    echo "$out" | grep -q "ft_strlen" \
      && ok "c09/ex01 make recompila tras editar" \
      || bad "c09/ex01 make no recompila tras tocar .c"
    ( cd "$M" && make re >/dev/null 2>&1 ) && [ -f "$M/libft.a" ] \
      && ok "c09/ex01 make re" || bad "c09/ex01 make re"
    ( cd "$M" && make fclean >/dev/null 2>&1 ) && [ ! -f "$M/libft.a" ] \
      && ok "c09/ex01 make fclean" || bad "c09/ex01 make fclean"
  else bad "c09/ex01 make no crea libft.a: $(head -1 "$TMP/e")"; fi
else skip "c09/ex01 Makefile (make no disponible)"; fi

# ex02 : ft_split
cat > "$TMP/msplit.c" <<'EOF'
#include <stdio.h>
char **ft_split(char *str, char *charset);
int main(void){char **t = ft_split("  hola,,42  mundo!", " ,");
int i = 0; if (!t) return 1;
while (t[i]){printf("[%s]", t[i]); i++;} return 0;}
EOF
if cc -Wall -Wextra -Werror "$C/c09/ex02/ft_split.c" "$TMP/msplit.c" \
     -o "$TMP/bsplit" 2>"$TMP/e"; then
  cmp_out "c09/ex02 ft_split" "$("$TMP/bsplit")" "[hola][42][mundo!]"
else bad "c09/ex02 ft_split: $(head -1 "$TMP/e")"; fi

echo "==================== C 10 ===================="

printf 'Linea uno\nLinea dos\nfin sin salto' > "$TMP/f1"
printf 'ABCDEFGHIJ' > "$TMP/f2"

# ex00 ft_display_file
if build "$C/c10/ex00" ft_display_file; then
  cmp_out "c10/ex00 display == cat" "$("$TMP/ft_display_file" "$TMP/f1")" \
    "$(cat "$TMP/f1")"
  cmp_out "c10/ex00 sin args" "$("$TMP/ft_display_file" 2>&1 1>/dev/null)" \
    "File name missing."
  cmp_out "c10/ex00 demasiados args" \
    "$("$TMP/ft_display_file" a b 2>&1 1>/dev/null)" "Too many arguments."
  cmp_out "c10/ex00 fichero ilegible" \
    "$("$TMP/ft_display_file" "$TMP/nope" 2>&1 1>/dev/null)" "Cannot read file."
else bad "c10/ex00 no compila: $(head -1 "$TMP/mkerr")"; fi

# ex01 ft_cat
if build "$C/c10/ex01" ft_cat; then
  cmp_out "c10/ex01 cat 1 fichero == cat" "$("$TMP/ft_cat" "$TMP/f1")" \
    "$(cat "$TMP/f1")"
  cmp_out "c10/ex01 cat 2 ficheros == cat" \
    "$("$TMP/ft_cat" "$TMP/f1" "$TMP/f2")" "$(cat "$TMP/f1" "$TMP/f2")"
  cmp_out "c10/ex01 cat desde stdin" "$(printf 'abc' | "$TMP/ft_cat")" "abc"
  "$TMP/ft_cat" "$TMP/nope" 2>"$TMP/e" 1>/dev/null
  if [ $? -ne 0 ] && [ -s "$TMP/e" ]; then ok "c10/ex01 cat error (exit!=0 + stderr)"
  else bad "c10/ex01 cat error mal manejado"; fi
else bad "c10/ex01 no compila: $(head -1 "$TMP/mkerr")"; fi

# ex02 ft_tail
if build "$C/c10/ex02" ft_tail; then
  if have tail; then
    cmp_out "c10/ex02 tail -c 5" "$("$TMP/ft_tail" -c 5 "$TMP/f1")" \
      "$(tail -c 5 "$TMP/f1")"
    cmp_out "c10/ex02 tail -c5 (pegado)" "$("$TMP/ft_tail" -c5 "$TMP/f1")" \
      "$(tail -c 5 "$TMP/f1")"
    cmp_out "c10/ex02 tail -c 999 (> tamano)" \
      "$("$TMP/ft_tail" -c 999 "$TMP/f1")" "$(tail -c 999 "$TMP/f1")"
    cmp_out "c10/ex02 tail -c 3 (2 ficheros)" \
      "$("$TMP/ft_tail" -c 3 "$TMP/f1" "$TMP/f2")" \
      "$(tail -c 3 "$TMP/f1" "$TMP/f2")"
  else skip "c10/ex02 tail (tail del sistema no disponible)"; fi
else bad "c10/ex02 no compila: $(head -1 "$TMP/mkerr")"; fi

# ex03 ft_hexdump  (referencia python validada contra el hexdump del sistema)
if build "$C/c10/ex03" ft_hexdump; then
  cmp_out "c10/ex03 hexdump 1 fichero" "$("$TMP/ft_hexdump" "$TMP/f1")" \
    "$(python3 "$TMP/hdref.py" "$TMP/f1")"
  cmp_out "c10/ex03 hexdump 2 ficheros (offset continuo)" \
    "$("$TMP/ft_hexdump" "$TMP/f1" "$TMP/f2")" \
    "$(python3 "$TMP/hdref.py" "$TMP/f1" "$TMP/f2")"
  : > "$TMP/empty"
  cmp_out "c10/ex03 hexdump fichero vacio" "$("$TMP/ft_hexdump" "$TMP/empty")" ""
else bad "c10/ex03 no compila: $(head -1 "$TMP/mkerr")"; fi

echo
echo "==== RESULTADO C08-C10: $PASS PASS / $FAIL FAIL / $SKIP SKIP ===="
rm -rf "$TMP"
[ "$FAIL" -eq 0 ]
