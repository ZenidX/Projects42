#!/bin/bash
# Compila cada ejercicio de C00/C01 con cc -Wall -Wextra -Werror + un main de
# test, ejecuta y compara salidas/resultados.
set -u
C=${C_ROOT:-/mnt/e/WORK/Xavi/Projects42/c}
TMP=$(mktemp -d)
PASS=0; FAIL=0
ok()  { echo "PASS: $1"; PASS=$((PASS+1)); }
bad() { echo "FAIL: $1"; FAIL=$((FAIL+1)); }

t() { # t <nombre> <fuente> <main.c> <esperado> [comparador]
  local name=$1 src=$2 main=$3 exp=$4
  echo "$main" > "$TMP/main.c"
  if ! cc -Wall -Wextra -Werror "$C/$src" "$TMP/main.c" -o "$TMP/prog" 2> "$TMP/err"; then
    bad "$name (no compila): $(head -2 "$TMP/err")"; return; fi
  local out; out=$("$TMP/prog")
  if [ "$out" = "$exp" ]; then ok "$name"; else bad "$name: [$out] != [$exp]"; fi
}

### C00
t "c00/ex00 ft_putchar" c00/ex00/ft_putchar.c \
  'void ft_putchar(char c); int main(void){ft_putchar(0x42);return 0;}' "B"

t "c00/ex01 alphabet" c00/ex01/ft_print_alphabet.c \
  'void ft_print_alphabet(void); int main(void){ft_print_alphabet();return 0;}' \
  "abcdefghijklmnopqrstuvwxyz"

t "c00/ex02 reverse alphabet" c00/ex02/ft_print_reverse_alphabet.c \
  'void ft_print_reverse_alphabet(void); int main(void){ft_print_reverse_alphabet();return 0;}' \
  "zyxwvutsrqponmlkjihgfedcba"

t "c00/ex03 numbers" c00/ex03/ft_print_numbers.c \
  'void ft_print_numbers(void); int main(void){ft_print_numbers();return 0;}' \
  "0123456789"

t "c00/ex04 is_negative" c00/ex04/ft_is_negative.c \
  'void ft_is_negative(int n); int main(void){ft_is_negative(-5);ft_is_negative(0);ft_is_negative(7);return 0;}' \
  "NPP"

# ex05: comparar contra generador de referencia en shell
exp05=$(python3 - <<'EOF'
combs = []
for a in range(8):
    for b in range(a+1, 9):
        for c in range(b+1, 10):
            combs.append(f"{a}{b}{c}")
print(", ".join(combs), end="")
EOF
)
t "c00/ex05 print_comb" c00/ex05/ft_print_comb.c \
  'void ft_print_comb(void); int main(void){ft_print_comb();return 0;}' "$exp05"

exp06=$(python3 - <<'EOF'
combs = []
for a in range(99):
    for b in range(a+1, 100):
        combs.append(f"{a:02d} {b:02d}")
print(", ".join(combs), end="")
EOF
)
t "c00/ex06 print_comb2" c00/ex06/ft_print_comb2.c \
  'void ft_print_comb2(void); int main(void){ft_print_comb2();return 0;}' "$exp06"

t "c00/ex07 putnbr" c00/ex07/ft_putnbr.c \
  'void ft_putnbr(int n); int main(void){ft_putnbr(42);ft_putnbr(0);ft_putnbr(-2147483648);ft_putnbr(2147483647);ft_putnbr(-7);return 0;}' \
  "420-21474836482147483647-7"

exp08=$(python3 - <<'EOF'
from itertools import combinations
print(", ".join("".join(map(str, c)) for c in combinations(range(10), 3)), end="")
EOF
)
t "c00/ex08 print_combn(3)" c00/ex08/ft_print_combn.c \
  'void ft_print_combn(int n); int main(void){ft_print_combn(3);return 0;}' "$exp08"

exp08b=$(python3 -c "from itertools import combinations; print(', '.join(''.join(map(str,c)) for c in combinations(range(10),1)), end='')")
t "c00/ex08 print_combn(1)" c00/ex08/ft_print_combn.c \
  'void ft_print_combn(int n); int main(void){ft_print_combn(1);return 0;}' "$exp08b"

### C01
t "c01/ex00 ft_ft" c01/ex00/ft_ft.c \
  '#include <stdio.h>
void ft_ft(int *n); int main(void){int x=0;ft_ft(&x);printf("%d",x);return 0;}' "42"

t "c01/ex01 ultimate_ft" c01/ex01/ft_ultimate_ft.c \
  '#include <stdio.h>
void ft_ultimate_ft(int *********n);
int main(void){int x=0;int *p1=&x;int **p2=&p1;int ***p3=&p2;int ****p4=&p3;int *****p5=&p4;
int ******p6=&p5;int *******p7=&p6;int ********p8=&p7;int *********p9=&p8;
ft_ultimate_ft(p9);printf("%d",x);return 0;}' "42"

t "c01/ex02 swap" c01/ex02/ft_swap.c \
  '#include <stdio.h>
void ft_swap(int *a, int *b); int main(void){int a=3,b=9;ft_swap(&a,&b);printf("%d %d",a,b);return 0;}' "9 3"

t "c01/ex03 div_mod" c01/ex03/ft_div_mod.c \
  '#include <stdio.h>
void ft_div_mod(int a, int b, int *d, int *m); int main(void){int d,m;ft_div_mod(17,5,&d,&m);printf("%d %d",d,m);return 0;}' "3 2"

t "c01/ex04 ultimate_div_mod" c01/ex04/ft_ultimate_div_mod.c \
  '#include <stdio.h>
void ft_ultimate_div_mod(int *a, int *b); int main(void){int a=17,b=5;ft_ultimate_div_mod(&a,&b);printf("%d %d",a,b);return 0;}' "3 2"

t "c01/ex05 putstr" c01/ex05/ft_putstr.c \
  'void ft_putstr(char *s); int main(void){ft_putstr("Hola 42");return 0;}' "Hola 42"

t "c01/ex06 strlen" c01/ex06/ft_strlen.c \
  '#include <stdio.h>
int ft_strlen(char *s); int main(void){printf("%d %d",ft_strlen(""),ft_strlen("hola mundo"));return 0;}' "0 10"

t "c01/ex07 rev_int_tab" c01/ex07/ft_rev_int_tab.c \
  '#include <stdio.h>
void ft_rev_int_tab(int *t, int s);
int main(void)
{
	int	t[5] = {1, 2, 3, 4, 5};
	int	u[4] = {9, 8, 7, 6};
	int	i;

	ft_rev_int_tab(t, 5);
	ft_rev_int_tab(u, 4);
	i = 0;
	while (i < 5)
		printf("%d", t[i++]);
	printf(" ");
	i = 0;
	while (i < 4)
		printf("%d", u[i++]);
	return (0);
}' "54321 6789"

t "c01/ex08 sort_int_tab" c01/ex08/ft_sort_int_tab.c \
  '#include <stdio.h>
void ft_sort_int_tab(int *t, int s); int main(void){int t[7]={5,-2,9,0,5,-8,3};int i;
ft_sort_int_tab(t,7);for(i=0;i<7;i++)printf("%d ",t[i]);return 0;}' "-8 -2 0 3 5 5 9 "

echo
echo "==== RESULTADO C: $PASS PASS / $FAIL FAIL ===="
rm -rf "$TMP"
