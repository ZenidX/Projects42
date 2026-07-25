#!/bin/bash
# Compila cada ejercicio de C02/C03/C04 con cc -Wall -Wextra -Werror + un main
# de test, ejecuta y compara salidas/resultados. La raiz de las fuentes es
# C_ROOT (por defecto /w/c dentro del contenedor con gcc).
set -u
C=${C_ROOT:-/w/c}
TMP=$(mktemp -d)
PASS=0; FAIL=0
ok()  { echo "PASS: $1"; PASS=$((PASS+1)); }
bad() { echo "FAIL: $1"; FAIL=$((FAIL+1)); }

t() { # t <nombre> <fuente> <main.c> <esperado>
  local name=$1 src=$2 main=$3 exp=$4
  printf '%s\n' "$main" > "$TMP/main.c"
  if ! cc -Wall -Wextra -Werror "$C/$src" "$TMP/main.c" -o "$TMP/prog" \
      2> "$TMP/err"; then
    bad "$name (no compila): $(head -2 "$TMP/err")"; return; fi
  local out; out=$("$TMP/prog")
  if [ "$out" = "$exp" ]; then ok "$name"; else bad "$name: [$out] != [$exp]"; fi
}

### ================= C 02 =================

t "c02/ex00 strcpy" c02/ex00/ft_strcpy.c \
'#include <stdio.h>
char *ft_strcpy(char *d, char *s);
int main(void)
{
	char	dest[32];

	ft_strcpy(dest, "42 Barcelona");
	printf("%s|", dest);
	printf("%s", ft_strcpy(dest, ""));
	return (0);
}' "42 Barcelona|"

t "c02/ex01 strncpy (padding)" c02/ex01/ft_strncpy.c \
'#include <stdio.h>
char *ft_strncpy(char *d, char *s, unsigned int n);
int main(void)
{
	char	dest[10];
	int		i;

	i = 0;
	while (i < 10)
	{
		dest[i] = '"'"'X'"'"';
		i++;
	}
	ft_strncpy(dest, "ab", 5);
	i = 0;
	while (i < 10)
	{
		printf("%c", dest[i] ? dest[i] : '"'"'.'"'"');
		i++;
	}
	return (0);
}' "ab...XXXXX"

t "c02/ex02 str_is_alpha" c02/ex02/ft_str_is_alpha.c \
'#include <stdio.h>
int ft_str_is_alpha(char *s);
int main(void)
{
	printf("%d%d%d%d", ft_str_is_alpha("abcZ"), ft_str_is_alpha("ab2"),
		ft_str_is_alpha(""), ft_str_is_alpha("a b"));
	return (0);
}' "1010"

t "c02/ex03 str_is_numeric" c02/ex03/ft_str_is_numeric.c \
'#include <stdio.h>
int ft_str_is_numeric(char *s);
int main(void)
{
	printf("%d%d%d%d", ft_str_is_numeric("0129"), ft_str_is_numeric("12a"),
		ft_str_is_numeric(""), ft_str_is_numeric("1 2"));
	return (0);
}' "1010"

t "c02/ex04 str_is_lowercase" c02/ex04/ft_str_is_lowercase.c \
'#include <stdio.h>
int ft_str_is_lowercase(char *s);
int main(void)
{
	printf("%d%d%d%d", ft_str_is_lowercase("abc"), ft_str_is_lowercase("aBc"),
		ft_str_is_lowercase(""), ft_str_is_lowercase("a1"));
	return (0);
}' "1010"

t "c02/ex05 str_is_uppercase" c02/ex05/ft_str_is_uppercase.c \
'#include <stdio.h>
int ft_str_is_uppercase(char *s);
int main(void)
{
	printf("%d%d%d%d", ft_str_is_uppercase("ABC"), ft_str_is_uppercase("AbC"),
		ft_str_is_uppercase(""), ft_str_is_uppercase("A1"));
	return (0);
}' "1010"

t "c02/ex06 str_is_printable" c02/ex06/ft_str_is_printable.c \
'#include <stdio.h>
int ft_str_is_printable(char *s);
int main(void)
{
	printf("%d%d%d", ft_str_is_printable("Hola 42!"),
		ft_str_is_printable("a\tb"), ft_str_is_printable(""));
	return (0);
}' "101"

t "c02/ex07 strupcase" c02/ex07/ft_strupcase.c \
'#include <stdio.h>
char *ft_strupcase(char *s);
int main(void)
{
	char	s[] = "Hola 42 mundo!";

	printf("%s", ft_strupcase(s));
	return (0);
}' "HOLA 42 MUNDO!"

t "c02/ex08 strlowcase" c02/ex08/ft_strlowcase.c \
'#include <stdio.h>
char *ft_strlowcase(char *s);
int main(void)
{
	char	s[] = "Hola 42 MUNDO!";

	printf("%s", ft_strlowcase(s));
	return (0);
}' "hola 42 mundo!"

t "c02/ex09 strcapitalize" c02/ex09/ft_strcapitalize.c \
'#include <stdio.h>
char *ft_strcapitalize(char *s);
int main(void)
{
	char	s[] = "salut, comment tu vas ? 42mots quarante-deux; cinquante+et+un";

	printf("%s", ft_strcapitalize(s));
	return (0);
}' "Salut, Comment Tu Vas ? 42mots Quarante-Deux; Cinquante+Et+Un"

t "c02/ex10 strlcpy" c02/ex10/ft_strlcpy.c \
'#include <stdio.h>
unsigned int ft_strlcpy(char *d, char *s, unsigned int size);
int main(void)
{
	char			dest[16];
	unsigned int	r;

	r = ft_strlcpy(dest, "Hello", 3);
	printf("%s %u|", dest, r);
	r = ft_strlcpy(dest, "Hi", 16);
	printf("%s %u", dest, r);
	return (0);
}' "He 5|Hi 2"

t "c02/ex11 putstr_non_printable" c02/ex11/ft_putstr_non_printable.c \
'void ft_putstr_non_printable(char *s);
int main(void)
{
	ft_putstr_non_printable("Coucou\ntu vas bien ?");
	return (0);
}' 'Coucou\0atu vas bien ?'

# ex12: la direccion es impredecible; se elimina el prefijo "<hex>: " de cada
# linea y se compara el resto (hex + ascii) contra un modelo generado en Python.
ex12_main='#include <unistd.h>
void *ft_print_memory(void *addr, unsigned int size);
int main(void)
{
	char	*s;

	s = "Bonjour a todos! 42\trules\n";
	ft_print_memory(s, 26);
	return (0);
}'
printf '%s\n' "$ex12_main" > "$TMP/main.c"
if cc -Wall -Wextra -Werror "$C/c02/ex12/ft_print_memory.c" "$TMP/main.c" \
    -o "$TMP/prog" 2> "$TMP/err"; then
  "$TMP/prog" | sed -E 's/^[0-9a-fA-F]+: //' > "$TMP/got"
  python3 - > "$TMP/exp" <<'PYEOF'
data = b"Bonjour a todos! 42\trules\n"
hexd = "0123456789abcdef"
out = ""
i = 0
while i < len(data):
    line = ""
    j = 0
    while j < 16:
        if i + j < len(data):
            b = data[i + j]
            line += hexd[b // 16] + hexd[b % 16]
        else:
            line += "  "
        if j % 2 == 1:
            line += " "
        j += 1
    j = 0
    while j < 16 and i + j < len(data):
        b = data[i + j]
        line += chr(b) if 32 <= b <= 126 else "."
        j += 1
    out += line + "\n"
    i += 16
import sys
sys.stdout.write(out)
PYEOF
  if diff -q "$TMP/got" "$TMP/exp" > /dev/null; then ok "c02/ex12 print_memory";
  else bad "c02/ex12 print_memory: salida != modelo"; fi
else
  bad "c02/ex12 print_memory (no compila): $(head -2 "$TMP/err")"
fi

### ================= C 03 =================

t "c03/ex00 strcmp" c03/ex00/ft_strcmp.c \
'#include <stdio.h>
int ft_strcmp(char *a, char *b);
int main(void)
{
	printf("%d %d %d %d", ft_strcmp("abc", "abc"), ft_strcmp("abc", "abd"),
		ft_strcmp("abd", "abc"), ft_strcmp("abc", "ab"));
	return (0);
}' "0 -1 1 99"

t "c03/ex01 strncmp" c03/ex01/ft_strncmp.c \
'#include <stdio.h>
int ft_strncmp(char *a, char *b, unsigned int n);
int main(void)
{
	printf("%d %d %d", ft_strncmp("abcX", "abcY", 3), ft_strncmp("abc", "abd", 5),
		ft_strncmp("abc", "abc", 0));
	return (0);
}' "0 -1 0"

t "c03/ex02 strcat" c03/ex02/ft_strcat.c \
'#include <stdio.h>
char *ft_strcat(char *d, char *s);
int main(void)
{
	char	d[32] = "Hello, ";

	printf("%s", ft_strcat(d, "42!"));
	return (0);
}' "Hello, 42!"

t "c03/ex03 strncat" c03/ex03/ft_strncat.c \
'#include <stdio.h>
char *ft_strncat(char *d, char *s, unsigned int nb);
int main(void)
{
	char	d[32] = "Hello, ";

	printf("%s", ft_strncat(d, "42world", 2));
	return (0);
}' "Hello, 42"

t "c03/ex04 strstr" c03/ex04/ft_strstr.c \
'#include <stdio.h>
char *ft_strstr(char *s, char *f);
int main(void)
{
	printf("%s|%d|%s", ft_strstr("hello world", "wor"),
		ft_strstr("abc", "xyz") == 0, ft_strstr("abc", ""));
	return (0);
}' "world|1|abc"

t "c03/ex05 strlcat" c03/ex05/ft_strlcat.c \
'#include <stdio.h>
unsigned int ft_strlcat(char *d, char *s, unsigned int size);
int main(void)
{
	char			a[16] = "Hello";
	char			b[4] = "AB";
	unsigned int	r;

	r = ft_strlcat(a, ", 42!", 16);
	printf("%s %u|", a, r);
	r = ft_strlcat(b, "cdef", 4);
	printf("%s %u", b, r);
	return (0);
}' "Hello, 42! 10|ABc 6"

### ================= C 04 =================

t "c04/ex00 strlen" c04/ex00/ft_strlen.c \
'#include <stdio.h>
int ft_strlen(char *s);
int main(void)
{
	printf("%d %d", ft_strlen(""), ft_strlen("Hola 42"));
	return (0);
}' "0 7"

t "c04/ex01 putstr" c04/ex01/ft_putstr.c \
'void ft_putstr(char *s);
int main(void)
{
	ft_putstr("42 rocks");
	return (0);
}' "42 rocks"

t "c04/ex02 putnbr" c04/ex02/ft_putnbr.c \
'#include <stdio.h>
void ft_putnbr(int n);
int main(void)
{
	setbuf(stdout, NULL);
	ft_putnbr(42);
	printf("|");
	ft_putnbr(-42);
	printf("|");
	ft_putnbr(0);
	printf("|");
	ft_putnbr(2147483647);
	printf("|");
	ft_putnbr(-2147483648);
	return (0);
}' "42|-42|0|2147483647|-2147483648"

t "c04/ex03 atoi" c04/ex03/ft_atoi.c \
'#include <stdio.h>
int ft_atoi(char *s);
int main(void)
{
	printf("%d|%d|%d|%d", ft_atoi(" ---+--+1234ab567"), ft_atoi("42"),
		ft_atoi("   -56x"), ft_atoi("+-+-13"));
	return (0);
}' "-1234|42|-56|13"

t "c04/ex04 putnbr_base" c04/ex04/ft_putnbr_base.c \
'#include <stdio.h>
void ft_putnbr_base(int nbr, char *base);
int main(void)
{
	setbuf(stdout, NULL);
	ft_putnbr_base(42, "0123456789");
	printf("|");
	ft_putnbr_base(42, "01");
	printf("|");
	ft_putnbr_base(-42, "0123456789ABCDEF");
	printf("|");
	ft_putnbr_base(0, "0123456789");
	printf("|");
	ft_putnbr_base(42, "+-");
	printf("|");
	ft_putnbr_base(42, "1");
	return (0);
}' "42|101010|-2A|0||"

t "c04/ex05 atoi_base" c04/ex05/ft_atoi_base.c \
'#include <stdio.h>
int ft_atoi_base(char *str, char *base);
int main(void)
{
	printf("%d|%d|%d|%d|%d|%d", ft_atoi_base("101010", "01"),
		ft_atoi_base("-2A", "0123456789ABCDEF"), ft_atoi_base("  ---5", "0123456789"),
		ft_atoi_base("FF", "0123456789ABCDEF"), ft_atoi_base("zz", "01"),
		ft_atoi_base("10", " 01"));
	return (0);
}' "42|-42|-5|255|0|0"

echo
echo "==== RESULTADO C02-C04: $PASS PASS / $FAIL FAIL ===="
rm -rf "$TMP"
