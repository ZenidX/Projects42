#!/usr/bin/env python3
"""Genera C08 (headers/structs), C09 (libft + Makefile + ft_split) y C10
(programas con ficheros) con header 42 valido y cuerpo conforme a la Norma.

Ejecutar:  wsl -d Ubuntu -- python3 /mnt/e/WORK/Xavi/Projects42/_build/gen_c08_10.py
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from header42 import write_file  # noqa: E402

ROOT = "/mnt/e/WORK/Xavi/Projects42/c"


def w(rel, body):
    """Fichero .c/.h con header 42."""
    write_file(ROOT, rel, body)


def raw(rel, text):
    """Fichero sin header 42 (Makefile, .sh); norminette no los revisa."""
    path = os.path.join(ROOT, rel)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", newline="\n") as f:
        f.write(text)
    print("OK " + rel)


PROG_MK = (
    "NAME = {name}\n\n"
    "CC = cc\n"
    "CFLAGS = -Wall -Wextra -Werror\n\n"
    "SRCS = {srcs}\n"
    "OBJS = $(SRCS:.c=.o)\n\n"
    "all: $(NAME)\n\n"
    "$(NAME): $(OBJS)\n"
    "\t$(CC) $(CFLAGS) $(OBJS) -o $(NAME)\n\n"
    "%.o: %.c\n"
    "\t$(CC) $(CFLAGS) -c $< -o $@\n\n"
    "clean:\n"
    "\trm -f $(OBJS)\n\n"
    "fclean: clean\n"
    "\trm -f $(NAME)\n\n"
    "re: fclean all\n\n"
    ".PHONY: all clean fclean re\n"
)


# ============================== C 08 ==============================
# ex00 : ft.h  (solo prototipos)
w("c08/ex00/ft.h",
"""#ifndef FT_H
# define FT_H

void\tft_putchar(char c);
void\tft_swap(int *a, int *b);
void\tft_putstr(char *str);
int\t\tft_strlen(char *str);
int\t\tft_strcmp(char *s1, char *s2);

#endif
""")

# ex01 : ft_boolean.h  (norminette -R CheckDefine)
w("c08/ex01/ft_boolean.h",
"""#ifndef FT_BOOLEAN_H
# define FT_BOOLEAN_H

# include <unistd.h>

# define EVEN_MSG "I have an even number of arguments.\\n"
# define ODD_MSG "I have an odd number of arguments.\\n"
# define SUCCESS 0
# define TRUE 1
# define FALSE 0
# define EVEN(nbr) ((nbr) % 2 == 0)

typedef int\tt_bool;

#endif
""")

# ex02 : ft_abs.h  (macro ABS)
w("c08/ex02/ft_abs.h",
"""#ifndef FT_ABS_H
# define FT_ABS_H

# define ABS(Value) (((Value) < 0) ? -(Value) : (Value))

#endif
""")

# ex03 : ft_point.h  (struct t_point)
w("c08/ex03/ft_point.h",
"""#ifndef FT_POINT_H
# define FT_POINT_H

typedef struct s_point
{
\tint\tx;
\tint\ty;
}\tt_point;

#endif
""")

# ex04 : ft_strs_to_tab.c  (+ ft_stock_str.h)
STOCK_H = (
"""#ifndef FT_STOCK_STR_H
# define FT_STOCK_STR_H

typedef struct s_stock_str
{
\tint\t\tsize;
\tchar\t*str;
\tchar\t*copy;
}\tt_stock_str;

#endif
""")
w("c08/ex04/ft_stock_str.h", STOCK_H)
w("c08/ex05/ft_stock_str.h", STOCK_H)

w("c08/ex04/ft_strs_to_tab.c",
"""#include "ft_stock_str.h"
#include <stdlib.h>

static int\tft_strlen(char *str)
{
\tint\ti;

\ti = 0;
\twhile (str[i])
\t\ti++;
\treturn (i);
}

static char\t*ft_strdup(char *src)
{
\tchar\t*dst;
\tint\t\ti;
\tint\t\tlen;

\tlen = ft_strlen(src);
\tdst = (char *)malloc(sizeof(char) * (len + 1));
\tif (!dst)
\t\treturn (0);
\ti = 0;
\twhile (i < len)
\t{
\t\tdst[i] = src[i];
\t\ti++;
\t}
\tdst[i] = '\\0';
\treturn (dst);
}

struct s_stock_str\t*ft_strs_to_tab(int ac, char **av)
{
\tstruct s_stock_str\t*tab;
\tint\t\t\t\t\ti;

\ttab = (struct s_stock_str *)malloc(sizeof(struct s_stock_str) * (ac + 1));
\tif (!tab)
\t\treturn (0);
\ti = 0;
\twhile (i < ac)
\t{
\t\ttab[i].size = ft_strlen(av[i]);
\t\ttab[i].str = av[i];
\t\ttab[i].copy = ft_strdup(av[i]);
\t\ti++;
\t}
\ttab[i].size = 0;
\ttab[i].str = 0;
\ttab[i].copy = 0;
\treturn (tab);
}
""")

# ex05 : ft_show_tab.c
w("c08/ex05/ft_show_tab.c",
"""#include "ft_stock_str.h"
#include <unistd.h>

void\tft_putstr(char *str)
{
\tint\ti;

\ti = 0;
\twhile (str[i])
\t{
\t\twrite(1, &str[i], 1);
\t\ti++;
\t}
}

void\tft_putnbr(int nb)
{
\tchar\tc;

\tif (nb < 0)
\t{
\t\twrite(1, "-", 1);
\t\tnb = -nb;
\t}
\tif (nb >= 10)
\t\tft_putnbr(nb / 10);
\tc = nb % 10 + '0';
\twrite(1, &c, 1);
}

void\tft_show_tab(struct s_stock_str *par)
{
\tint\ti;

\ti = 0;
\twhile (par[i].str)
\t{
\t\tft_putstr(par[i].str);
\t\twrite(1, "\\n", 1);
\t\tft_putnbr(par[i].size);
\t\twrite(1, "\\n", 1);
\t\tft_putstr(par[i].copy);
\t\twrite(1, "\\n", 1);
\t\ti++;
\t}
}
""")


# ============================== C 09 ==============================
# ex00 : libft  (5 fuentes + libft_creator.sh)
w("c09/ex00/ft_putchar.c",
"""#include <unistd.h>

void\tft_putchar(char c)
{
\twrite(1, &c, 1);
}
""")

w("c09/ex00/ft_swap.c",
"""void\tft_swap(int *a, int *b)
{
\tint\ttmp;

\ttmp = *a;
\t*a = *b;
\t*b = tmp;
}
""")

w("c09/ex00/ft_putstr.c",
"""#include <unistd.h>

void\tft_putstr(char *str)
{
\tint\ti;

\ti = 0;
\twhile (str[i])
\t{
\t\twrite(1, &str[i], 1);
\t\ti++;
\t}
}
""")

w("c09/ex00/ft_strlen.c",
"""int\tft_strlen(char *str)
{
\tint\ti;

\ti = 0;
\twhile (str[i])
\t\ti++;
\treturn (i);
}
""")

w("c09/ex00/ft_strcmp.c",
"""int\tft_strcmp(char *s1, char *s2)
{
\tint\ti;

\ti = 0;
\twhile (s1[i] && s1[i] == s2[i])
\t\ti++;
\treturn ((unsigned char)s1[i] - (unsigned char)s2[i]);
}
""")

raw("c09/ex00/libft_creator.sh",
"""#!/bin/sh
cc -Wall -Wextra -Werror -c ft_putchar.c ft_swap.c ft_putstr.c ft_strlen.c \\
\tft_strcmp.c
ar rcs libft.a ft_putchar.o ft_swap.o ft_putstr.o ft_strlen.o ft_strcmp.o
rm -f ft_putchar.o ft_swap.o ft_putstr.o ft_strlen.o ft_strcmp.o
""")

# ex01 : Makefile  (unico entregable; fuentes en srcs/, header en includes/)
raw("c09/ex01/Makefile",
"""NAME = libft.a

CC = cc
CFLAGS = -Wall -Wextra -Werror
INCLUDES = -I includes

SRCS =\tsrcs/ft_putchar.c \\
\t\tsrcs/ft_swap.c \\
\t\tsrcs/ft_putstr.c \\
\t\tsrcs/ft_strlen.c \\
\t\tsrcs/ft_strcmp.c
OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
\tar rcs $(NAME) $(OBJS)

%.o: %.c
\t$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
\trm -f $(OBJS)

fclean: clean
\trm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
""")

# ex02 : ft_split.c
w("c09/ex02/ft_split.c",
"""#include <stdlib.h>

static int\tft_is_sep(char c, char *charset)
{
\tint\ti;

\ti = 0;
\twhile (charset[i])
\t{
\t\tif (charset[i] == c)
\t\t\treturn (1);
\t\ti++;
\t}
\treturn (0);
}

static int\tft_count_words(char *str, char *charset)
{
\tint\ti;
\tint\twords;

\ti = 0;
\twords = 0;
\twhile (str[i])
\t{
\t\twhile (str[i] && ft_is_sep(str[i], charset))
\t\t\ti++;
\t\tif (str[i] && !ft_is_sep(str[i], charset))
\t\t\twords++;
\t\twhile (str[i] && !ft_is_sep(str[i], charset))
\t\t\ti++;
\t}
\treturn (words);
}

static char\t*ft_dup_word(char *str, char *charset)
{
\tint\t\tlen;
\tint\t\ti;
\tchar\t*word;

\tlen = 0;
\twhile (str[len] && !ft_is_sep(str[len], charset))
\t\tlen++;
\tword = (char *)malloc(sizeof(char) * (len + 1));
\tif (!word)
\t\treturn (0);
\ti = 0;
\twhile (i < len)
\t{
\t\tword[i] = str[i];
\t\ti++;
\t}
\tword[i] = '\\0';
\treturn (word);
}

static int\tft_fill(char **tab, char *str, char *charset)
{
\tint\ti;
\tint\tw;

\ti = 0;
\tw = 0;
\twhile (str[i])
\t{
\t\twhile (str[i] && ft_is_sep(str[i], charset))
\t\t\ti++;
\t\tif (str[i])
\t\t{
\t\t\ttab[w] = ft_dup_word(str + i, charset);
\t\t\tif (!tab[w])
\t\t\t\treturn (0);
\t\t\tw++;
\t\t}
\t\twhile (str[i] && !ft_is_sep(str[i], charset))
\t\t\ti++;
\t}
\ttab[w] = 0;
\treturn (1);
}

char\t**ft_split(char *str, char *charset)
{
\tchar\t**tab;

\ttab = (char **)malloc(sizeof(char *) * (ft_count_words(str, charset) + 1));
\tif (!tab)
\t\treturn (0);
\tif (!ft_fill(tab, str, charset))
\t{
\t\tfree(tab);
\t\treturn (0);
\t}
\treturn (tab);
}
""")


# ============================== C 10 ==============================
# ex00 : ft_display_file
raw("c10/ex00/Makefile", PROG_MK.format(name="ft_display_file",
                                        srcs="ft_display_file.c"))
w("c10/ex00/ft_display_file.c",
"""#include <unistd.h>
#include <fcntl.h>

static int\tft_strlen(char *s)
{
\tint\ti;

\ti = 0;
\twhile (s[i])
\t\ti++;
\treturn (i);
}

static void\tft_putstr_fd(char *s, int fd)
{
\twrite(fd, s, ft_strlen(s));
}

static void\tft_read_file(int fd)
{
\tchar\tbuf[4096];
\tint\t\tn;

\tn = read(fd, buf, 4096);
\twhile (n > 0)
\t{
\t\twrite(1, buf, n);
\t\tn = read(fd, buf, 4096);
\t}
}

int\tmain(int argc, char **argv)
{
\tint\tfd;

\tif (argc < 2)
\t{
\t\tft_putstr_fd("File name missing.\\n", 2);
\t\treturn (1);
\t}
\tif (argc > 2)
\t{
\t\tft_putstr_fd("Too many arguments.\\n", 2);
\t\treturn (1);
\t}
\tfd = open(argv[1], O_RDONLY);
\tif (fd < 0)
\t{
\t\tft_putstr_fd("Cannot read file.\\n", 2);
\t\treturn (1);
\t}
\tft_read_file(fd);
\tclose(fd);
\treturn (0);
}
""")

# ex01 : ft_cat
raw("c10/ex01/Makefile", PROG_MK.format(name="ft_cat", srcs="ft_cat.c"))
w("c10/ex01/ft_cat.c",
"""#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>

static int\tft_strlen(char *s)
{
\tint\ti;

\ti = 0;
\twhile (s[i])
\t\ti++;
\treturn (i);
}

static void\tft_putstr_fd(char *s, int fd)
{
\twrite(fd, s, ft_strlen(s));
}

static void\tft_error(char *prog, char *file)
{
\tft_putstr_fd(prog, 2);
\tft_putstr_fd(": ", 2);
\tft_putstr_fd(file, 2);
\tft_putstr_fd(": ", 2);
\tft_putstr_fd(strerror(errno), 2);
\tft_putstr_fd("\\n", 2);
}

static void\tft_cat_fd(int fd)
{
\tchar\tbuf[4096];
\tint\t\tn;

\tn = read(fd, buf, 4096);
\twhile (n > 0)
\t{
\t\twrite(1, buf, n);
\t\tn = read(fd, buf, 4096);
\t}
}

int\tmain(int argc, char **argv)
{
\tint\ti;
\tint\tfd;
\tint\tstatus;

\tstatus = 0;
\ti = 1;
\tif (argc == 1)
\t\tft_cat_fd(0);
\twhile (i < argc)
\t{
\t\tfd = open(argv[i], O_RDONLY);
\t\tif (fd < 0)
\t\t{
\t\t\tft_error(basename(argv[0]), argv[i]);
\t\t\tstatus = 1;
\t\t}
\t\telse
\t\t{
\t\t\tft_cat_fd(fd);
\t\t\tclose(fd);
\t\t}
\t\ti++;
\t}
\treturn (status);
}
""")

# ex02 : ft_tail  (ft_tail.c + ft_tail_utils.c + ft_tail.h)
raw("c10/ex02/Makefile", PROG_MK.format(name="ft_tail",
                                        srcs="ft_tail.c ft_tail_utils.c"))
w("c10/ex02/ft_tail.h",
"""#ifndef FT_TAIL_H
# define FT_TAIL_H

# include <unistd.h>
# include <fcntl.h>
# include <errno.h>
# include <string.h>
# include <libgen.h>

int\t\tft_strlen(char *s);
void\tft_putstr_fd(char *s, int fd);
void\tft_error(char *prog, char *file);
int\t\tft_atoi(char *s);
void\tft_header(char *file);
int\t\tft_file_size(int fd);
void\tft_print_last(int fd, int skip);
int\t\tft_process(char *prog, char *file, int count);
int\t\tft_parse(char **argv, int argc, int *count);

#endif
""")

w("c10/ex02/ft_tail_utils.c",
"""#include "ft_tail.h"

int\tft_strlen(char *s)
{
\tint\ti;

\ti = 0;
\twhile (s[i])
\t\ti++;
\treturn (i);
}

void\tft_putstr_fd(char *s, int fd)
{
\twrite(fd, s, ft_strlen(s));
}

void\tft_error(char *prog, char *file)
{
\tft_putstr_fd(prog, 2);
\tft_putstr_fd(": ", 2);
\tft_putstr_fd(file, 2);
\tft_putstr_fd(": ", 2);
\tft_putstr_fd(strerror(errno), 2);
\tft_putstr_fd("\\n", 2);
}

int\tft_atoi(char *s)
{
\tint\tn;
\tint\ti;

\tn = 0;
\ti = 0;
\twhile (s[i] >= '0' && s[i] <= '9')
\t{
\t\tn = n * 10 + (s[i] - '0');
\t\ti++;
\t}
\treturn (n);
}

void\tft_header(char *file)
{
\tft_putstr_fd("==> ", 1);
\tft_putstr_fd(file, 1);
\tft_putstr_fd(" <==\\n", 1);
}
""")

w("c10/ex02/ft_tail.c",
"""#include "ft_tail.h"

int\tft_file_size(int fd)
{
\tchar\tbuf[4096];
\tint\t\ttotal;
\tint\t\tn;

\ttotal = 0;
\tn = read(fd, buf, 4096);
\twhile (n > 0)
\t{
\t\ttotal = total + n;
\t\tn = read(fd, buf, 4096);
\t}
\treturn (total);
}

void\tft_print_last(int fd, int skip)
{
\tchar\tbuf[4096];
\tint\t\tn;
\tint\t\tpos;

\tpos = 0;
\tn = read(fd, buf, 4096);
\twhile (n > 0)
\t{
\t\tif (pos + n > skip)
\t\t{
\t\t\tif (pos >= skip)
\t\t\t\twrite(1, buf, n);
\t\t\telse
\t\t\t\twrite(1, buf + (skip - pos), n - (skip - pos));
\t\t}
\t\tpos = pos + n;
\t\tn = read(fd, buf, 4096);
\t}
}

int\tft_process(char *prog, char *file, int count)
{
\tint\tfd;
\tint\tsize;
\tint\tskip;

\tfd = open(file, O_RDONLY);
\tif (fd < 0)
\t{
\t\tft_error(prog, file);
\t\treturn (1);
\t}
\tsize = ft_file_size(fd);
\tclose(fd);
\tskip = 0;
\tif (count < size)
\t\tskip = size - count;
\tfd = open(file, O_RDONLY);
\tft_print_last(fd, skip);
\tclose(fd);
\treturn (0);
}

int\tft_parse(char **argv, int argc, int *count)
{
\tint\ti;

\ti = 1;
\twhile (i < argc)
\t{
\t\tif (argv[i][0] == '-' && argv[i][1] == 'c')
\t\t{
\t\t\tif (argv[i][2])
\t\t\t\t*count = ft_atoi(argv[i] + 2);
\t\t\telse
\t\t\t{
\t\t\t\ti++;
\t\t\t\t*count = ft_atoi(argv[i]);
\t\t\t}
\t\t\treturn (i + 1);
\t\t}
\t\ti++;
\t}
\treturn (-1);
}

int\tmain(int argc, char **argv)
{
\tint\tcount;
\tint\ti;
\tint\tmulti;
\tint\tprinted;

\tcount = 0;
\ti = ft_parse(argv, argc, &count);
\tif (i < 0)
\t\treturn (1);
\tmulti = (argc - i > 1);
\tprinted = 0;
\twhile (i < argc)
\t{
\t\tif (multi)
\t\t{
\t\t\tif (printed)
\t\t\t\tft_putstr_fd("\\n", 1);
\t\t\tft_header(argv[i]);
\t\t}
\t\tft_process(basename(argv[0]), argv[i], count);
\t\tprinted = 1;
\t\ti++;
\t}
\treturn (0);
}
""")

# ex03 : ft_hexdump  (ft_hexdump.c + ft_hexdump_utils.c + ft_hexdump.h)
raw("c10/ex03/Makefile", PROG_MK.format(name="ft_hexdump",
                                        srcs="ft_hexdump.c ft_hexdump_utils.c"))
w("c10/ex03/ft_hexdump.h",
"""#ifndef FT_HEXDUMP_H
# define FT_HEXDUMP_H

# include <unistd.h>
# include <fcntl.h>
# include <errno.h>
# include <string.h>
# include <libgen.h>

typedef struct s_dump
{
\tunsigned char\tline[16];
\tint\t\t\t\tlen;
\tint\t\t\t\toffset;
}\tt_dump;

int\t\tft_strlen(char *s);
void\tft_putstr_fd(char *s, int fd);
void\tft_error(char *prog, char *file);
void\tft_puthex(unsigned int value, int digits);
void\tft_put_group(unsigned char b0, unsigned char b1);
void\tft_print_line(unsigned char *line, int len, int offset);
void\tft_feed(t_dump *d, unsigned char *buf, int n);
void\tft_dump_fd(int fd, t_dump *d);
int\t\tft_dump_files(int argc, char **argv, t_dump *d);

#endif
""")

w("c10/ex03/ft_hexdump_utils.c",
"""#include "ft_hexdump.h"

int\tft_strlen(char *s)
{
\tint\ti;

\ti = 0;
\twhile (s[i])
\t\ti++;
\treturn (i);
}

void\tft_putstr_fd(char *s, int fd)
{
\twrite(fd, s, ft_strlen(s));
}

void\tft_error(char *prog, char *file)
{
\tft_putstr_fd(prog, 2);
\tft_putstr_fd(": ", 2);
\tft_putstr_fd(file, 2);
\tft_putstr_fd(": ", 2);
\tft_putstr_fd(strerror(errno), 2);
\tft_putstr_fd("\\n", 2);
}

void\tft_puthex(unsigned int value, int digits)
{
\tchar\thex[16];
\tint\t\ti;
\tchar\t*base;

\tbase = "0123456789abcdef";
\ti = digits;
\twhile (i > 0)
\t{
\t\ti--;
\t\thex[i] = base[value & 15];
\t\tvalue = value >> 4;
\t}
\twrite(1, hex, digits);
}

void\tft_put_group(unsigned char b0, unsigned char b1)
{
\tft_puthex((unsigned int)b1, 2);
\tft_puthex((unsigned int)b0, 2);
}
""")

w("c10/ex03/ft_hexdump.c",
"""#include "ft_hexdump.h"

void\tft_print_line(unsigned char *line, int len, int offset)
{
\tint\ti;
\tint\tpad;

\tft_puthex((unsigned int)offset, 7);
\ti = 0;
\twhile (i < len)
\t{
\t\tft_putstr_fd(" ", 1);
\t\tif (i + 1 < len)
\t\t\tft_put_group(line[i], line[i + 1]);
\t\telse
\t\t\tft_put_group(line[i], 0);
\t\ti += 2;
\t}
\tpad = (8 - (len + 1) / 2) * 5;
\twhile (pad > 0)
\t{
\t\tft_putstr_fd(" ", 1);
\t\tpad--;
\t}
\tft_putstr_fd("\\n", 1);
}

void\tft_feed(t_dump *d, unsigned char *buf, int n)
{
\tint\ti;

\ti = 0;
\twhile (i < n)
\t{
\t\td->line[d->len] = buf[i];
\t\td->len++;
\t\tif (d->len == 16)
\t\t{
\t\t\tft_print_line(d->line, 16, d->offset);
\t\t\td->offset = d->offset + 16;
\t\t\td->len = 0;
\t\t}
\t\ti++;
\t}
}

void\tft_dump_fd(int fd, t_dump *d)
{
\tunsigned char\tbuf[4096];
\tint\t\t\t\tn;

\tn = read(fd, buf, 4096);
\twhile (n > 0)
\t{
\t\tft_feed(d, buf, n);
\t\tn = read(fd, buf, 4096);
\t}
}

int\tft_dump_files(int argc, char **argv, t_dump *d)
{
\tint\ti;
\tint\tfd;
\tint\tstatus;

\tstatus = 0;
\ti = 1;
\twhile (i < argc)
\t{
\t\tfd = open(argv[i], O_RDONLY);
\t\tif (fd < 0)
\t\t{
\t\t\tft_error(basename(argv[0]), argv[i]);
\t\t\tstatus = 1;
\t\t}
\t\telse
\t\t{
\t\t\tft_dump_fd(fd, d);
\t\t\tclose(fd);
\t\t}
\t\ti++;
\t}
\treturn (status);
}

int\tmain(int argc, char **argv)
{
\tt_dump\td;
\tint\t\tstatus;

\td.len = 0;
\td.offset = 0;
\tif (argc == 1)
\t{
\t\tft_dump_fd(0, &d);
\t\tstatus = 0;
\t}
\telse
\t\tstatus = ft_dump_files(argc, argv, &d);
\tif (d.len > 0)
\t\tft_print_line(d.line, d.len, d.offset);
\tif (d.offset + d.len > 0)
\t{
\t\tft_puthex((unsigned int)(d.offset + d.len), 7);
\t\tft_putstr_fd("\\n", 1);
\t}
\treturn (status);
}
""")

print("\nGeneracion C08-C10 completada.")
