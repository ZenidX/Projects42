#!/bin/bash
# Compila cada ejercicio de C05/C06/C07 con cc -Wall -Wextra -Werror y valida
# salidas/valores de retorno. Pensado para correr en docker (con gcc).
# C05/C07 son funciones -> se enlazan con un main de test.
# C06 son PROGRAMAS -> se compila el .c solo y se le pasan argumentos.
set -u
C=${C_ROOT:-/w/c}
TMP=$(mktemp -d)
PASS=0; FAIL=0
ok()  { echo "PASS: $1"; PASS=$((PASS+1)); }
bad() { echo "FAIL: $1"; FAIL=$((FAIL+1)); }

t() { # t <nombre> <fuentes> <main.c> <esperado>
  local name=$1 srcs=$2 main=$3 exp=$4
  local files="" s
  echo "$main" > "$TMP/main.c"
  for s in $srcs; do files="$files $C/$s"; done
  if ! cc -Wall -Wextra -Werror $files "$TMP/main.c" -o "$TMP/prog" 2> "$TMP/err"; then
    bad "$name (no compila): $(head -2 "$TMP/err")"; return; fi
  local out; out=$("$TMP/prog")
  if [ "$out" = "$exp" ]; then ok "$name"; else bad "$name: [$out] != [$exp]"; fi
}

tp() { # tp <nombre> <fuente> <esperado> [args...]  (programa con su propio main)
  local name=$1 src=$2 exp=$3; shift 3
  if ! cc -Wall -Wextra -Werror "$C/$src" -o "$TMP/prog" 2> "$TMP/err"; then
    bad "$name (no compila): $(head -2 "$TMP/err")"; return; fi
  local out; out=$("$TMP/prog" "$@")
  if [ "$out" = "$exp" ]; then ok "$name"; else bad "$name: [$out] != [$exp]"; fi
}

echo "==== C05 ===="
t "c05/ex00 iterative_factorial" c05/ex00/ft_iterative_factorial.c \
  '#include <stdio.h>
int ft_iterative_factorial(int n);
int main(void){printf("%d %d %d %d %d",ft_iterative_factorial(0),
ft_iterative_factorial(1),ft_iterative_factorial(5),
ft_iterative_factorial(4),ft_iterative_factorial(-3));return 0;}' \
  "1 1 120 24 0"

t "c05/ex01 recursive_factorial" c05/ex01/ft_recursive_factorial.c \
  '#include <stdio.h>
int ft_recursive_factorial(int n);
int main(void){printf("%d %d %d %d %d",ft_recursive_factorial(0),
ft_recursive_factorial(1),ft_recursive_factorial(5),
ft_recursive_factorial(4),ft_recursive_factorial(-3));return 0;}' \
  "1 1 120 24 0"

t "c05/ex02 iterative_power" c05/ex02/ft_iterative_power.c \
  '#include <stdio.h>
int ft_iterative_power(int n,int p);
int main(void){printf("%d %d %d %d %d",ft_iterative_power(2,10),
ft_iterative_power(5,0),ft_iterative_power(0,0),
ft_iterative_power(2,3),ft_iterative_power(3,-2));return 0;}' \
  "1024 1 1 8 0"

t "c05/ex03 recursive_power" c05/ex03/ft_recursive_power.c \
  '#include <stdio.h>
int ft_recursive_power(int n,int p);
int main(void){printf("%d %d %d %d %d",ft_recursive_power(2,10),
ft_recursive_power(5,0),ft_recursive_power(0,0),
ft_recursive_power(2,3),ft_recursive_power(3,-2));return 0;}' \
  "1024 1 1 8 0"

t "c05/ex04 fibonacci" c05/ex04/ft_fibonacci.c \
  '#include <stdio.h>
int ft_fibonacci(int i);
int main(void){printf("%d %d %d %d %d %d",ft_fibonacci(0),ft_fibonacci(1),
ft_fibonacci(2),ft_fibonacci(9),ft_fibonacci(-1),ft_fibonacci(10));return 0;}' \
  "0 1 1 34 -1 55"

t "c05/ex05 sqrt" c05/ex05/ft_sqrt.c \
  '#include <stdio.h>
int ft_sqrt(int n);
int main(void){printf("%d %d %d %d %d %d",ft_sqrt(0),ft_sqrt(1),ft_sqrt(4),
ft_sqrt(2),ft_sqrt(25),ft_sqrt(2147395600));return 0;}' \
  "0 1 2 0 5 46340"

t "c05/ex06 is_prime" c05/ex06/ft_is_prime.c \
  '#include <stdio.h>
int ft_is_prime(int n);
int main(void){printf("%d%d%d%d%d%d%d",ft_is_prime(0),ft_is_prime(1),
ft_is_prime(2),ft_is_prime(7),ft_is_prime(9),ft_is_prime(-5),
ft_is_prime(7919));return 0;}' \
  "0011001"

t "c05/ex07 find_next_prime" c05/ex07/ft_find_next_prime.c \
  '#include <stdio.h>
int ft_find_next_prime(int n);
int main(void){printf("%d %d %d %d %d",ft_find_next_prime(0),
ft_find_next_prime(14),ft_find_next_prime(17),
ft_find_next_prime(20),ft_find_next_prime(-5));return 0;}' \
  "2 17 17 23 2"

# ex08: programa que imprime soluciones por stdout y el retorno por stderr.
cat > "$TMP/main.c" <<'EOF'
#include <stdio.h>
int ft_ten_queens_puzzle(void);
int main(void){int n=ft_ten_queens_puzzle();fprintf(stderr,"%d",n);return 0;}
EOF
if cc -Wall -Wextra -Werror "$C/c05/ex08/ft_ten_queens_puzzle.c" "$TMP/main.c" \
    -o "$TMP/prog" 2> "$TMP/err"; then
  qout=$("$TMP/prog" 2> "$TMP/ret"); qret=$(cat "$TMP/ret")
  qlines=$(printf '%s\n' "$qout" | grep -c '^[0-9]\{10\}$')
  qfirst=$(printf '%s\n' "$qout" | head -1)
  if [ "$qret" = "724" ] && [ "$qlines" = "724" ] && [ "$qfirst" = "0257948136" ]; then
    ok "c05/ex08 ten_queens (ret=$qret, lineas=$qlines, 1a=$qfirst)"
  else
    bad "c05/ex08 ten_queens: ret=$qret lineas=$qlines 1a=$qfirst"
  fi
else
  bad "c05/ex08 ten_queens (no compila): $(head -2 "$TMP/err")"
fi

echo "==== C06 (programas) ===="
tp "c06/ex00 print_program_name" c06/ex00/ft_print_program_name.c "$TMP/prog"
tp "c06/ex01 print_params" c06/ex01/ft_print_params.c $'test1\ntest2\ntest3' \
  test1 test2 test3
tp "c06/ex02 rev_params" c06/ex02/ft_rev_params.c $'test3\ntest2\ntest1' \
  test1 test2 test3
tp "c06/ex03 sort_params" c06/ex03/ft_sort_params.c $'apple\nbanana\ncherry' \
  banana cherry apple
tp "c06/ex03 sort_params (ASCII)" c06/ex03/ft_sort_params.c $'ABC\nabc\nzoo' \
  zoo abc ABC
tp "c06/ex01 sin args" c06/ex01/ft_print_params.c ""

echo "==== C07 ===="
t "c07/ex00 strdup" c07/ex00/ft_strdup.c \
  '#include <stdio.h>
#include <stdlib.h>
char *ft_strdup(char *src);
int main(void){char *s="Naheulbeuk";char *d=ft_strdup(s);
if(!d){printf("NULL");return 1;}printf("%s|%d",d,d!=s);free(d);return 0;}' \
  "Naheulbeuk|1"

t "c07/ex01 range" c07/ex01/ft_range.c \
  '#include <stdio.h>
#include <stdlib.h>
int *ft_range(int min,int max);
int main(void){int *r=ft_range(1,5);int i;for(i=0;i<4;i++)printf("%d",r[i]);
free(r);printf("|");r=ft_range(-2,2);for(i=0;i<4;i++)printf("%d ",r[i]);free(r);
printf("|%d",ft_range(5,1)==NULL);return 0;}' \
  "1234|-2 -1 0 1 |1"

t "c07/ex02 ultimate_range" c07/ex02/ft_ultimate_range.c \
  '#include <stdio.h>
#include <stdlib.h>
int ft_ultimate_range(int **range,int min,int max);
int main(void)
{
	int	*r;
	int	n;
	int	i;

	n = ft_ultimate_range(&r, 1, 5);
	printf("%d:", n);
	i = 0;
	while (i < n)
		printf("%d", r[i++]);
	free(r);
	printf("|");
	n = ft_ultimate_range(&r, 5, 5);
	printf("%d:%d", n, r == NULL);
	return (0);
}' \
  "4:1234|0:1"

t "c07/ex03 strjoin" c07/ex03/ft_strjoin.c \
  '#include <stdio.h>
#include <stdlib.h>
char *ft_strjoin(int size,char **strs,char *sep);
int main(void){char *a[3]={"orc","goblin","troll"};
char *r=ft_strjoin(3,a,", ");printf("%s",r);free(r);printf("|");
r=ft_strjoin(0,a,", ");printf("[%s]",r);free(r);printf("|");
char *b[1]={"solo"};r=ft_strjoin(1,b,"-");printf("%s",r);free(r);return 0;}' \
  "orc, goblin, troll|[]|solo"

t "c07/ex04 convert_base" "c07/ex04/ft_convert_base.c c07/ex04/ft_convert_base2.c" \
  '#include <stdio.h>
#include <stdlib.h>
char *ft_convert_base(char *nbr,char *bf,char *bt);
int main(void){char *r;
r=ft_convert_base("42","0123456789","01");printf("%s|",r);free(r);
r=ft_convert_base("  -42","0123456789","0123456789");printf("%s|",r);free(r);
r=ft_convert_base("ff","0123456789abcdef","0123456789");printf("%s|",r);free(r);
r=ft_convert_base("--42","0123456789","0123456789");printf("%s|",r);free(r);
r=ft_convert_base("0","0123456789","0123456789");printf("%s|",r);free(r);
printf("%d",ft_convert_base("42","0123456789","0")==NULL);return 0;}' \
  "101010|-42|255|42|0|1"

t "c07/ex05 split" c07/ex05/ft_split.c \
  '#include <stdio.h>
#include <stdlib.h>
char **ft_split(char *str,char *charset);
int main(void){char **r=ft_split("  hello   world  foo ","  ");int i=0;
while(r[i]){printf("[%s]",r[i]);free(r[i]);i++;}free(r);printf("|%d|",i);
r=ft_split("aXXbYcXY","XY");i=0;while(r[i]){printf("[%s]",r[i]);free(r[i]);i++;}
free(r);printf("%d",i);return 0;}' \
  "[hello][world][foo]|3|[a][b][c]3"

echo
echo "==== RESULTADO C05-C07: $PASS PASS / $FAIL FAIL ===="
rm -rf "$TMP"
