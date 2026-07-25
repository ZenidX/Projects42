#!/usr/bin/env python3
"""Genera los .c de C05/C06/C07 con header 42 y cuerpo conforme a la Norma.

Uso: wsl -d Ubuntu -- python3 /mnt/e/WORK/Xavi/Projects42/_build/gen_c05_07.py
"""
from header42 import write_file

ROOT = "/mnt/e/WORK/Xavi/Projects42/c"
T = "\t"
F = {}

# ============================== C 05 ==============================
# ex00: ft_iterative_factorial
F["c05/ex00/ft_iterative_factorial.c"] = f"""int{T}ft_iterative_factorial(int nb)
{{
{T}int{T}result;
{T}int{T}i;

{T}if (nb < 0)
{T}{T}return (0);
{T}result = 1;
{T}i = 1;
{T}while (i <= nb)
{T}{{
{T}{T}result *= i;
{T}{T}i++;
{T}}}
{T}return (result);
}}
"""

# ex01: ft_recursive_factorial
F["c05/ex01/ft_recursive_factorial.c"] = f"""int{T}ft_recursive_factorial(int nb)
{{
{T}if (nb < 0)
{T}{T}return (0);
{T}if (nb <= 1)
{T}{T}return (1);
{T}return (nb * ft_recursive_factorial(nb - 1));
}}
"""

# ex02: ft_iterative_power
F["c05/ex02/ft_iterative_power.c"] = f"""int{T}ft_iterative_power(int nb, int power)
{{
{T}int{T}result;

{T}if (power < 0)
{T}{T}return (0);
{T}result = 1;
{T}while (power > 0)
{T}{{
{T}{T}result *= nb;
{T}{T}power--;
{T}}}
{T}return (result);
}}
"""

# ex03: ft_recursive_power
F["c05/ex03/ft_recursive_power.c"] = f"""int{T}ft_recursive_power(int nb, int power)
{{
{T}if (power < 0)
{T}{T}return (0);
{T}if (power == 0)
{T}{T}return (1);
{T}return (nb * ft_recursive_power(nb, power - 1));
}}
"""

# ex04: ft_fibonacci
F["c05/ex04/ft_fibonacci.c"] = f"""int{T}ft_fibonacci(int index)
{{
{T}if (index < 0)
{T}{T}return (-1);
{T}if (index == 0)
{T}{T}return (0);
{T}if (index == 1)
{T}{T}return (1);
{T}return (ft_fibonacci(index - 1) + ft_fibonacci(index - 2));
}}
"""

# ex05: ft_sqrt
F["c05/ex05/ft_sqrt.c"] = f"""int{T}ft_sqrt(int nb)
{{
{T}long{T}i;

{T}if (nb < 1)
{T}{T}return (0);
{T}i = 1;
{T}while (i * i < nb)
{T}{T}i++;
{T}if (i * i == nb)
{T}{T}return (i);
{T}return (0);
}}
"""

# ex06: ft_is_prime
F["c05/ex06/ft_is_prime.c"] = f"""int{T}ft_is_prime(int nb)
{{
{T}long{T}i;

{T}if (nb < 2)
{T}{T}return (0);
{T}i = 2;
{T}while (i * i <= nb)
{T}{{
{T}{T}if (nb % i == 0)
{T}{T}{T}return (0);
{T}{T}i++;
{T}}}
{T}return (1);
}}
"""

# ex07: ft_find_next_prime
F["c05/ex07/ft_find_next_prime.c"] = f"""static int{T}ft_is_prime(int nb)
{{
{T}long{T}i;

{T}if (nb < 2)
{T}{T}return (0);
{T}i = 2;
{T}while (i * i <= nb)
{T}{{
{T}{T}if (nb % i == 0)
{T}{T}{T}return (0);
{T}{T}i++;
{T}}}
{T}return (1);
}}

int{T}ft_find_next_prime(int nb)
{{
{T}while (!ft_is_prime(nb))
{T}{T}nb++;
{T}return (nb);
}}
"""

# ex08: ft_ten_queens_puzzle
F["c05/ex08/ft_ten_queens_puzzle.c"] = f"""#include <unistd.h>

static int{T}is_valid(int *board, int col, int row)
{{
{T}int{T}i;

{T}i = 0;
{T}while (i < col)
{T}{{
{T}{T}if (board[i] == row)
{T}{T}{T}return (0);
{T}{T}if (col - i == row - board[i] || col - i == board[i] - row)
{T}{T}{T}return (0);
{T}{T}i++;
{T}}}
{T}return (1);
}}

static void{T}print_board(int *board)
{{
{T}char{T}line[11];
{T}int{T}{T}i;

{T}i = 0;
{T}while (i < 10)
{T}{{
{T}{T}line[i] = board[i] + '0';
{T}{T}i++;
{T}}}
{T}line[10] = '\\n';
{T}write(1, line, 11);
}}

static int{T}solve(int *board, int col)
{{
{T}int{T}row;
{T}int{T}count;

{T}if (col == 10)
{T}{{
{T}{T}print_board(board);
{T}{T}return (1);
{T}}}
{T}row = 0;
{T}count = 0;
{T}while (row < 10)
{T}{{
{T}{T}if (is_valid(board, col, row))
{T}{T}{{
{T}{T}{T}board[col] = row;
{T}{T}{T}count += solve(board, col + 1);
{T}{T}}}
{T}{T}row++;
{T}}}
{T}return (count);
}}

int{T}ft_ten_queens_puzzle(void)
{{
{T}int{T}board[10];

{T}return (solve(board, 0));
}}
"""

# ============================== C 06 ==============================
# ex00: ft_print_program_name (programa con main)
F["c06/ex00/ft_print_program_name.c"] = f"""#include <unistd.h>

int{T}main(int argc, char **argv)
{{
{T}int{T}i;

{T}(void)argc;
{T}i = 0;
{T}while (argv[0][i])
{T}{{
{T}{T}write(1, &argv[0][i], 1);
{T}{T}i++;
{T}}}
{T}write(1, "\\n", 1);
{T}return (0);
}}
"""

# ex01: ft_print_params
F["c06/ex01/ft_print_params.c"] = f"""#include <unistd.h>

static void{T}print_str(char *s)
{{
{T}int{T}i;

{T}i = 0;
{T}while (s[i])
{T}{{
{T}{T}write(1, &s[i], 1);
{T}{T}i++;
{T}}}
{T}write(1, "\\n", 1);
}}

int{T}main(int argc, char **argv)
{{
{T}int{T}i;

{T}i = 1;
{T}while (i < argc)
{T}{{
{T}{T}print_str(argv[i]);
{T}{T}i++;
{T}}}
{T}return (0);
}}
"""

# ex02: ft_rev_params
F["c06/ex02/ft_rev_params.c"] = f"""#include <unistd.h>

static void{T}print_str(char *s)
{{
{T}int{T}i;

{T}i = 0;
{T}while (s[i])
{T}{{
{T}{T}write(1, &s[i], 1);
{T}{T}i++;
{T}}}
{T}write(1, "\\n", 1);
}}

int{T}main(int argc, char **argv)
{{
{T}int{T}i;

{T}i = argc - 1;
{T}while (i >= 1)
{T}{{
{T}{T}print_str(argv[i]);
{T}{T}i--;
{T}}}
{T}return (0);
}}
"""

# ex03: ft_sort_params
F["c06/ex03/ft_sort_params.c"] = f"""#include <unistd.h>

static void{T}print_str(char *s)
{{
{T}int{T}i;

{T}i = 0;
{T}while (s[i])
{T}{{
{T}{T}write(1, &s[i], 1);
{T}{T}i++;
{T}}}
{T}write(1, "\\n", 1);
}}

static int{T}ft_strcmp(char *a, char *b)
{{
{T}int{T}i;

{T}i = 0;
{T}while (a[i] && a[i] == b[i])
{T}{T}i++;
{T}return ((unsigned char)a[i] - (unsigned char)b[i]);
}}

static void{T}sort(char **tab, int size)
{{
{T}char{T}*tmp;
{T}int{T}{T}i;

{T}i = 1;
{T}while (i < size)
{T}{{
{T}{T}if (ft_strcmp(tab[i - 1], tab[i]) > 0)
{T}{T}{{
{T}{T}{T}tmp = tab[i - 1];
{T}{T}{T}tab[i - 1] = tab[i];
{T}{T}{T}tab[i] = tmp;
{T}{T}{T}i = 1;
{T}{T}}}
{T}{T}else
{T}{T}{T}i++;
{T}}}
}}

int{T}main(int argc, char **argv)
{{
{T}int{T}i;

{T}sort(argv + 1, argc - 1);
{T}i = 1;
{T}while (i < argc)
{T}{{
{T}{T}print_str(argv[i]);
{T}{T}i++;
{T}}}
{T}return (0);
}}
"""

# ============================== C 07 ==============================
# ex00: ft_strdup
F["c07/ex00/ft_strdup.c"] = f"""#include <stdlib.h>

static int{T}ft_strlen(char *s)
{{
{T}int{T}i;

{T}i = 0;
{T}while (s[i])
{T}{T}i++;
{T}return (i);
}}

char{T}*ft_strdup(char *src)
{{
{T}char{T}*dst;
{T}int{T}{T}i;

{T}dst = (char *)malloc(sizeof(char) * (ft_strlen(src) + 1));
{T}if (!dst)
{T}{T}return (NULL);
{T}i = 0;
{T}while (src[i])
{T}{{
{T}{T}dst[i] = src[i];
{T}{T}i++;
{T}}}
{T}dst[i] = '\\0';
{T}return (dst);
}}
"""

# ex01: ft_range
F["c07/ex01/ft_range.c"] = f"""#include <stdlib.h>

int{T}*ft_range(int min, int max)
{{
{T}int{T}*arr;
{T}int{T}i;
{T}int{T}size;

{T}if (min >= max)
{T}{T}return (NULL);
{T}size = max - min;
{T}arr = (int *)malloc(sizeof(int) * size);
{T}if (!arr)
{T}{T}return (NULL);
{T}i = 0;
{T}while (i < size)
{T}{{
{T}{T}arr[i] = min + i;
{T}{T}i++;
{T}}}
{T}return (arr);
}}
"""

# ex02: ft_ultimate_range
F["c07/ex02/ft_ultimate_range.c"] = f"""#include <stdlib.h>

int{T}ft_ultimate_range(int **range, int min, int max)
{{
{T}int{T}i;
{T}int{T}size;

{T}if (min >= max)
{T}{{
{T}{T}*range = NULL;
{T}{T}return (0);
{T}}}
{T}size = max - min;
{T}*range = (int *)malloc(sizeof(int) * size);
{T}if (!*range)
{T}{T}return (-1);
{T}i = 0;
{T}while (i < size)
{T}{{
{T}{T}(*range)[i] = min + i;
{T}{T}i++;
{T}}}
{T}return (size);
}}
"""

# ex03: ft_strjoin
F["c07/ex03/ft_strjoin.c"] = f"""#include <stdlib.h>

static int{T}ft_strlen(char *s)
{{
{T}int{T}i;

{T}i = 0;
{T}while (s[i])
{T}{T}i++;
{T}return (i);
}}

static int{T}total_len(int size, char **strs, char *sep)
{{
{T}int{T}len;
{T}int{T}i;

{T}if (size == 0)
{T}{T}return (0);
{T}len = ft_strlen(sep) * (size - 1);
{T}i = 0;
{T}while (i < size)
{T}{{
{T}{T}len += ft_strlen(strs[i]);
{T}{T}i++;
{T}}}
{T}return (len);
}}

static char{T}*ft_append(char *dst, char *src)
{{
{T}int{T}i;

{T}i = 0;
{T}while (src[i])
{T}{{
{T}{T}*dst = src[i];
{T}{T}dst++;
{T}{T}i++;
{T}}}
{T}return (dst);
}}

char{T}*ft_strjoin(int size, char **strs, char *sep)
{{
{T}char{T}*res;
{T}char{T}*p;
{T}int{T}{T}i;

{T}res = (char *)malloc(sizeof(char) * (total_len(size, strs, sep) + 1));
{T}if (!res)
{T}{T}return (NULL);
{T}p = res;
{T}i = 0;
{T}while (i < size)
{T}{{
{T}{T}p = ft_append(p, strs[i]);
{T}{T}if (i < size - 1)
{T}{T}{T}p = ft_append(p, sep);
{T}{T}i++;
{T}}}
{T}*p = '\\0';
{T}return (res);
}}
"""

# ex04: ft_convert_base.c + ft_convert_base2.c
F["c07/ex04/ft_convert_base2.c"] = f"""int{T}ft_is_space(char c)
{{
{T}return (c == ' ' || (c >= 9 && c <= 13));
}}

int{T}ft_baselen(char *base)
{{
{T}int{T}i;

{T}i = 0;
{T}while (base[i])
{T}{T}i++;
{T}return (i);
}}

int{T}ft_is_valid_base(char *base)
{{
{T}int{T}i;
{T}int{T}j;

{T}if (ft_baselen(base) < 2)
{T}{T}return (0);
{T}i = 0;
{T}while (base[i])
{T}{{
{T}{T}if (ft_is_space(base[i]) || base[i] == '+' || base[i] == '-')
{T}{T}{T}return (0);
{T}{T}j = i + 1;
{T}{T}while (base[j])
{T}{T}{{
{T}{T}{T}if (base[i] == base[j])
{T}{T}{T}{T}return (0);
{T}{T}{T}j++;
{T}{T}}}
{T}{T}i++;
{T}}}
{T}return (1);
}}

int{T}ft_index_in_base(char *base, char c)
{{
{T}int{T}i;

{T}i = 0;
{T}while (base[i])
{T}{{
{T}{T}if (base[i] == c)
{T}{T}{T}return (i);
{T}{T}i++;
{T}}}
{T}return (-1);
}}

long{T}ft_atoi_base(char *nbr, char *base)
{{
{T}long{T}result;
{T}int{T}{T}sign;
{T}int{T}{T}digit;
{T}int{T}{T}len;

{T}result = 0;
{T}sign = 1;
{T}len = ft_baselen(base);
{T}while (ft_is_space(*nbr))
{T}{T}nbr++;
{T}while (*nbr == '+' || *nbr == '-')
{T}{{
{T}{T}if (*nbr == '-')
{T}{T}{T}sign = -sign;
{T}{T}nbr++;
{T}}}
{T}digit = ft_index_in_base(base, *nbr);
{T}while (digit >= 0)
{T}{{
{T}{T}result = result * len + digit;
{T}{T}nbr++;
{T}{T}digit = ft_index_in_base(base, *nbr);
{T}}}
{T}return (result * sign);
}}
"""

F["c07/ex04/ft_convert_base.c"] = f"""#include <stdlib.h>

int{T}{T}ft_is_valid_base(char *base);
long{T}ft_atoi_base(char *nbr, char *base);
int{T}{T}ft_baselen(char *base);

static int{T}nbr_len(long n, int base_len)
{{
{T}int{T}len;

{T}len = 1;
{T}while (n >= base_len)
{T}{{
{T}{T}n /= base_len;
{T}{T}len++;
{T}}}
{T}return (len);
}}

static char{T}*fill_result(long n, char *base, int base_len)
{{
{T}char{T}*res;
{T}int{T}{T}len;
{T}int{T}{T}neg;

{T}neg = (n < 0);
{T}if (neg)
{T}{T}n = -n;
{T}len = nbr_len(n, base_len) + neg;
{T}res = (char *)malloc(sizeof(char) * (len + 1));
{T}if (!res)
{T}{T}return (NULL);
{T}res[len] = '\\0';
{T}while (len > neg)
{T}{{
{T}{T}len--;
{T}{T}res[len] = base[n % base_len];
{T}{T}n /= base_len;
{T}}}
{T}if (neg)
{T}{T}res[0] = '-';
{T}return (res);
}}

char{T}*ft_convert_base(char *nbr, char *base_from, char *base_to)
{{
{T}long{T}value;

{T}if (!ft_is_valid_base(base_from) || !ft_is_valid_base(base_to))
{T}{T}return (NULL);
{T}value = ft_atoi_base(nbr, base_from);
{T}return (fill_result(value, base_to, ft_baselen(base_to)));
}}
"""

# ex05: ft_split
F["c07/ex05/ft_split.c"] = f"""#include <stdlib.h>

static int{T}is_sep(char c, char *charset)
{{
{T}int{T}i;

{T}i = 0;
{T}while (charset[i])
{T}{{
{T}{T}if (charset[i] == c)
{T}{T}{T}return (1);
{T}{T}i++;
{T}}}
{T}return (0);
}}

static int{T}count_words(char *str, char *charset)
{{
{T}int{T}count;
{T}int{T}in_word;
{T}int{T}i;

{T}count = 0;
{T}in_word = 0;
{T}i = 0;
{T}while (str[i])
{T}{{
{T}{T}if (is_sep(str[i], charset))
{T}{T}{T}in_word = 0;
{T}{T}else if (!in_word)
{T}{T}{{
{T}{T}{T}in_word = 1;
{T}{T}{T}count++;
{T}{T}}}
{T}{T}i++;
{T}}}
{T}return (count);
}}

static char{T}*word_dup(char *str, int start, int end)
{{
{T}char{T}*word;
{T}int{T}{T}i;

{T}word = (char *)malloc(sizeof(char) * (end - start + 1));
{T}if (!word)
{T}{T}return (NULL);
{T}i = 0;
{T}while (start < end)
{T}{{
{T}{T}word[i] = str[start];
{T}{T}i++;
{T}{T}start++;
{T}}}
{T}word[i] = '\\0';
{T}return (word);
}}

static char{T}**fill_words(char **result, char *str, char *charset)
{{
{T}int{T}i;
{T}int{T}start;
{T}int{T}w;

{T}i = 0;
{T}w = 0;
{T}while (str[i])
{T}{{
{T}{T}if (!is_sep(str[i], charset))
{T}{T}{{
{T}{T}{T}start = i;
{T}{T}{T}while (str[i] && !is_sep(str[i], charset))
{T}{T}{T}{T}i++;
{T}{T}{T}result[w] = word_dup(str, start, i);
{T}{T}{T}w++;
{T}{T}}}
{T}{T}else
{T}{T}{T}i++;
{T}}}
{T}result[w] = NULL;
{T}return (result);
}}

char{T}**ft_split(char *str, char *charset)
{{
{T}char{T}**result;

{T}result = (char **)malloc(sizeof(char *) * (count_words(str, charset) + 1));
{T}if (!result)
{T}{T}return (NULL);
{T}return (fill_words(result, str, charset));
}}
"""

for rel, body in F.items():
    write_file(ROOT, rel, body)
print(f"\n{len(F)} archivos generados")
