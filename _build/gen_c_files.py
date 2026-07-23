#!/usr/bin/env python3
"""Genera los .c de C00/C01 con header 42 valido y cuerpo conforme a la Norma."""
import os

ROOT = "/mnt/e/WORK/Xavi/Projects42/c"
LOGIN = "xalara"
EMAIL = "xalara@student.42barcelona.com"
DATE = "2026/07/23 12:00:00"

TPL = [
    "/* ************************************************************************** */",
    "/*                                                                            */",
    "/*                                                        :::      ::::::::   */",
    "/*                                                      :+:      :+:    :+:   */",
    "/*                                                    +:+ +:+         +:+     */",
    "/*                                                  +#+  +:+       +#+        */",
    "/*                                                +#+#+#+#+#+   +#+           */",
    "/*                                                     #+#    #+#             */",
    "/*                                                    ###   ########.fr       */",
    "/*                                                                            */",
    "/* ************************************************************************** */",
]

def splice(line, text):
    out = "/*   " + text + line[5 + len(text):]
    assert len(out) == 80, f"header line !=80: {len(out)} [{text}]"
    return out

def header(fname):
    l = TPL[:]
    l[3] = splice(l[3], fname)
    l[5] = splice(l[5], f"By: {LOGIN} <{EMAIL}>")
    l[7] = splice(l[7], f"Created: {DATE} by {LOGIN}")
    l[8] = splice(l[8], f"Updated: {DATE} by {LOGIN}")
    return "\n".join(l) + "\n\n"

T = "\t"
F = {}

# ============================== C 00 ==============================
F["c00/ex00/ft_putchar.c"] = f"""#include <unistd.h>

void{T}ft_putchar(char c)
{{
{T}write(1, &c, 1);
}}
"""

F["c00/ex01/ft_print_alphabet.c"] = f"""#include <unistd.h>

void{T}ft_print_alphabet(void)
{{
{T}char{T}c;

{T}c = 'a';
{T}while (c <= 'z')
{T}{{
{T}{T}write(1, &c, 1);
{T}{T}c++;
{T}}}
}}
"""

F["c00/ex02/ft_print_reverse_alphabet.c"] = f"""#include <unistd.h>

void{T}ft_print_reverse_alphabet(void)
{{
{T}char{T}c;

{T}c = 'z';
{T}while (c >= 'a')
{T}{{
{T}{T}write(1, &c, 1);
{T}{T}c--;
{T}}}
}}
"""

F["c00/ex03/ft_print_numbers.c"] = f"""#include <unistd.h>

void{T}ft_print_numbers(void)
{{
{T}char{T}c;

{T}c = '0';
{T}while (c <= '9')
{T}{{
{T}{T}write(1, &c, 1);
{T}{T}c++;
{T}}}
}}
"""

F["c00/ex04/ft_is_negative.c"] = f"""#include <unistd.h>

void{T}ft_is_negative(int n)
{{
{T}if (n < 0)
{T}{T}write(1, "N", 1);
{T}else
{T}{T}write(1, "P", 1);
}}
"""

F["c00/ex05/ft_print_comb.c"] = f"""#include <unistd.h>

static void{T}ft_print_trio(char a, char b, char c)
{{
{T}write(1, &a, 1);
{T}write(1, &b, 1);
{T}write(1, &c, 1);
{T}if (!(a == '7' && b == '8' && c == '9'))
{T}{T}write(1, ", ", 2);
}}

void{T}ft_print_comb(void)
{{
{T}char{T}a;
{T}char{T}b;
{T}char{T}c;

{T}a = '0';
{T}while (a <= '7')
{T}{{
{T}{T}b = a + 1;
{T}{T}while (b <= '8')
{T}{T}{{
{T}{T}{T}c = b + 1;
{T}{T}{T}while (c <= '9')
{T}{T}{T}{{
{T}{T}{T}{T}ft_print_trio(a, b, c);
{T}{T}{T}{T}c++;
{T}{T}{T}}}
{T}{T}{T}b++;
{T}{T}}}
{T}{T}a++;
{T}}}
}}
"""

F["c00/ex06/ft_print_comb2.c"] = f"""#include <unistd.h>

static void{T}ft_print_pair(int a, int b)
{{
{T}char{T}s[5];

{T}s[0] = a / 10 + '0';
{T}s[1] = a % 10 + '0';
{T}s[2] = ' ';
{T}s[3] = b / 10 + '0';
{T}s[4] = b % 10 + '0';
{T}write(1, s, 5);
{T}if (!(a == 98 && b == 99))
{T}{T}write(1, ", ", 2);
}}

void{T}ft_print_comb2(void)
{{
{T}int{T}a;
{T}int{T}b;

{T}a = 0;
{T}while (a <= 98)
{T}{{
{T}{T}b = a + 1;
{T}{T}while (b <= 99)
{T}{T}{{
{T}{T}{T}ft_print_pair(a, b);
{T}{T}{T}b++;
{T}{T}}}
{T}{T}a++;
{T}}}
}}
"""

F["c00/ex07/ft_putnbr.c"] = f"""#include <unistd.h>

void{T}ft_putnbr(int nb)
{{
{T}char{T}c;
{T}long{T}n;

{T}n = nb;
{T}if (n < 0)
{T}{{
{T}{T}write(1, "-", 1);
{T}{T}n = -n;
{T}}}
{T}if (n >= 10)
{T}{T}ft_putnbr(n / 10);
{T}c = n % 10 + '0';
{T}write(1, &c, 1);
}}
"""

F["c00/ex08/ft_print_combn.c"] = f"""#include <unistd.h>

static void{T}ft_print_tab(int *tab, int n)
{{
{T}char{T}c;
{T}int{T}{T}i;

{T}i = 0;
{T}while (i < n)
{T}{{
{T}{T}c = tab[i] + '0';
{T}{T}write(1, &c, 1);
{T}{T}i++;
{T}}}
{T}if (tab[0] != 10 - n)
{T}{T}write(1, ", ", 2);
}}

static void{T}ft_rec(int *tab, int pos, int n, int start)
{{
{T}int{T}d;

{T}if (pos == n)
{T}{{
{T}{T}ft_print_tab(tab, n);
{T}{T}return ;
{T}}}
{T}d = start;
{T}while (d <= 9)
{T}{{
{T}{T}tab[pos] = d;
{T}{T}ft_rec(tab, pos + 1, n, d + 1);
{T}{T}d++;
{T}}}
}}

void{T}ft_print_combn(int n)
{{
{T}int{T}tab[10];

{T}ft_rec(tab, 0, n, 0);
}}
"""

# ============================== C 01 ==============================
F["c01/ex00/ft_ft.c"] = f"""void{T}ft_ft(int *nbr)
{{
{T}*nbr = 42;
}}
"""

F["c01/ex01/ft_ultimate_ft.c"] = f"""void{T}ft_ultimate_ft(int *********nbr)
{{
{T}*********nbr = 42;
}}
"""

F["c01/ex02/ft_swap.c"] = f"""void{T}ft_swap(int *a, int *b)
{{
{T}int{T}tmp;

{T}tmp = *a;
{T}*a = *b;
{T}*b = tmp;
}}
"""

F["c01/ex03/ft_div_mod.c"] = f"""void{T}ft_div_mod(int a, int b, int *div, int *mod)
{{
{T}*div = a / b;
{T}*mod = a % b;
}}
"""

F["c01/ex04/ft_ultimate_div_mod.c"] = f"""void{T}ft_ultimate_div_mod(int *a, int *b)
{{
{T}int{T}div;
{T}int{T}mod;

{T}div = *a / *b;
{T}mod = *a % *b;
{T}*a = div;
{T}*b = mod;
}}
"""

F["c01/ex05/ft_putstr.c"] = f"""#include <unistd.h>

void{T}ft_putstr(char *str)
{{
{T}int{T}i;

{T}i = 0;
{T}while (str[i])
{T}{{
{T}{T}write(1, &str[i], 1);
{T}{T}i++;
{T}}}
}}
"""

F["c01/ex06/ft_strlen.c"] = f"""int{T}ft_strlen(char *str)
{{
{T}int{T}i;

{T}i = 0;
{T}while (str[i])
{T}{T}i++;
{T}return (i);
}}
"""

F["c01/ex07/ft_rev_int_tab.c"] = f"""void{T}ft_rev_int_tab(int *tab, int size)
{{
{T}int{T}i;
{T}int{T}tmp;

{T}i = 0;
{T}while (i < size / 2)
{T}{{
{T}{T}tmp = tab[i];
{T}{T}tab[i] = tab[size - 1 - i];
{T}{T}tab[size - 1 - i] = tmp;
{T}{T}i++;
{T}}}
}}
"""

F["c01/ex08/ft_sort_int_tab.c"] = f"""void{T}ft_sort_int_tab(int *tab, int size)
{{
{T}int{T}i;
{T}int{T}tmp;

{T}i = 0;
{T}while (i < size - 1)
{T}{{
{T}{T}if (tab[i] > tab[i + 1])
{T}{T}{{
{T}{T}{T}tmp = tab[i];
{T}{T}{T}tab[i] = tab[i + 1];
{T}{T}{T}tab[i + 1] = tmp;
{T}{T}{T}i = 0;
{T}{T}}}
{T}{T}else
{T}{T}{T}i++;
{T}}}
}}
"""

for rel, body in F.items():
    path = os.path.join(ROOT, rel)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", newline="\n") as f:
        f.write(header(os.path.basename(rel)) + body)
    print(f"OK {rel}")
print(f"\n{len(F)} archivos generados")
