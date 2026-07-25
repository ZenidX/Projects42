#!/usr/bin/env python3
"""Genera los .c de C02/C03/C04 con header 42 y cuerpo conforme a la Norma."""
from header42 import write_file

ROOT = "/mnt/e/WORK/Xavi/Projects42/c"
T = "\t"
F = {}

# ============================== C 02 ==============================

F["c02/ex00/ft_strcpy.c"] = f"""char{T}*ft_strcpy(char *dest, char *src)
{{
{T}int{T}i;

{T}i = 0;
{T}while (src[i])
{T}{{
{T}{T}dest[i] = src[i];
{T}{T}i++;
{T}}}
{T}dest[i] = '\\0';
{T}return (dest);
}}
"""

F["c02/ex01/ft_strncpy.c"] = f"""char{T}*ft_strncpy(char *dest, char *src, unsigned int n)
{{
{T}unsigned int{T}i;

{T}i = 0;
{T}while (i < n && src[i])
{T}{{
{T}{T}dest[i] = src[i];
{T}{T}i++;
{T}}}
{T}while (i < n)
{T}{{
{T}{T}dest[i] = '\\0';
{T}{T}i++;
{T}}}
{T}return (dest);
}}
"""

F["c02/ex02/ft_str_is_alpha.c"] = f"""int{T}ft_str_is_alpha(char *str)
{{
{T}int{T}i;

{T}i = 0;
{T}while (str[i])
{T}{{
{T}{T}if (str[i] < 'A' || (str[i] > 'Z' && str[i] < 'a') || str[i] > 'z')
{T}{T}{T}return (0);
{T}{T}i++;
{T}}}
{T}return (1);
}}
"""

F["c02/ex03/ft_str_is_numeric.c"] = f"""int{T}ft_str_is_numeric(char *str)
{{
{T}int{T}i;

{T}i = 0;
{T}while (str[i])
{T}{{
{T}{T}if (str[i] < '0' || str[i] > '9')
{T}{T}{T}return (0);
{T}{T}i++;
{T}}}
{T}return (1);
}}
"""

F["c02/ex04/ft_str_is_lowercase.c"] = f"""int{T}ft_str_is_lowercase(char *str)
{{
{T}int{T}i;

{T}i = 0;
{T}while (str[i])
{T}{{
{T}{T}if (str[i] < 'a' || str[i] > 'z')
{T}{T}{T}return (0);
{T}{T}i++;
{T}}}
{T}return (1);
}}
"""

F["c02/ex05/ft_str_is_uppercase.c"] = f"""int{T}ft_str_is_uppercase(char *str)
{{
{T}int{T}i;

{T}i = 0;
{T}while (str[i])
{T}{{
{T}{T}if (str[i] < 'A' || str[i] > 'Z')
{T}{T}{T}return (0);
{T}{T}i++;
{T}}}
{T}return (1);
}}
"""

F["c02/ex06/ft_str_is_printable.c"] = f"""int{T}ft_str_is_printable(char *str)
{{
{T}int{T}i;

{T}i = 0;
{T}while (str[i])
{T}{{
{T}{T}if (str[i] < 32 || str[i] > 126)
{T}{T}{T}return (0);
{T}{T}i++;
{T}}}
{T}return (1);
}}
"""

F["c02/ex07/ft_strupcase.c"] = f"""char{T}*ft_strupcase(char *str)
{{
{T}int{T}i;

{T}i = 0;
{T}while (str[i])
{T}{{
{T}{T}if (str[i] >= 'a' && str[i] <= 'z')
{T}{T}{T}str[i] = str[i] - 32;
{T}{T}i++;
{T}}}
{T}return (str);
}}
"""

F["c02/ex08/ft_strlowcase.c"] = f"""char{T}*ft_strlowcase(char *str)
{{
{T}int{T}i;

{T}i = 0;
{T}while (str[i])
{T}{{
{T}{T}if (str[i] >= 'A' && str[i] <= 'Z')
{T}{T}{T}str[i] = str[i] + 32;
{T}{T}i++;
{T}}}
{T}return (str);
}}
"""

F["c02/ex09/ft_strcapitalize.c"] = f"""static int{T}ft_is_alnum(char c)
{{
{T}if (c >= '0' && c <= '9')
{T}{T}return (1);
{T}if (c >= 'a' && c <= 'z')
{T}{T}return (1);
{T}if (c >= 'A' && c <= 'Z')
{T}{T}return (1);
{T}return (0);
}}

char{T}*ft_strcapitalize(char *str)
{{
{T}int{T}i;

{T}i = 0;
{T}while (str[i])
{T}{{
{T}{T}if (i != 0 && ft_is_alnum(str[i - 1]))
{T}{T}{{
{T}{T}{T}if (str[i] >= 'A' && str[i] <= 'Z')
{T}{T}{T}{T}str[i] = str[i] + 32;
{T}{T}}}
{T}{T}else if (str[i] >= 'a' && str[i] <= 'z')
{T}{T}{T}str[i] = str[i] - 32;
{T}{T}i++;
{T}}}
{T}return (str);
}}
"""

F["c02/ex10/ft_strlcpy.c"] = f"""unsigned int{T}ft_strlcpy(char *dest, char *src, unsigned int size)
{{
{T}unsigned int{T}i;
{T}unsigned int{T}len;

{T}len = 0;
{T}while (src[len])
{T}{T}len++;
{T}i = 0;
{T}while (i + 1 < size && src[i])
{T}{{
{T}{T}dest[i] = src[i];
{T}{T}i++;
{T}}}
{T}if (size > 0)
{T}{T}dest[i] = '\\0';
{T}return (len);
}}
"""

F["c02/ex11/ft_putstr_non_printable.c"] = f"""#include <unistd.h>

void{T}ft_putstr_non_printable(char *str)
{{
{T}char{T}*hex;
{T}int{T}{T}i;

{T}hex = "0123456789abcdef";
{T}i = 0;
{T}while (str[i])
{T}{{
{T}{T}if (str[i] >= 32 && str[i] <= 126)
{T}{T}{T}write(1, &str[i], 1);
{T}{T}else
{T}{T}{{
{T}{T}{T}write(1, "\\\\", 1);
{T}{T}{T}write(1, &hex[(unsigned char)str[i] / 16], 1);
{T}{T}{T}write(1, &hex[(unsigned char)str[i] % 16], 1);
{T}{T}}}
{T}{T}i++;
{T}}}
}}
"""

F["c02/ex12/ft_print_memory.c"] = f"""#include <unistd.h>

static void{T}ft_addr(unsigned long n)
{{
{T}char{T}*h;
{T}char{T}buf[16];
{T}int{T}{T}i;

{T}h = "0123456789abcdef";
{T}i = 16;
{T}while (i > 0)
{T}{{
{T}{T}i--;
{T}{T}buf[i] = h[n % 16];
{T}{T}n = n / 16;
{T}}}
{T}write(1, buf, 16);
{T}write(1, ": ", 2);
}}

static void{T}ft_hexline(unsigned char *p, unsigned int start, unsigned int size)
{{
{T}char{T}*h;
{T}int{T}{T}j;

{T}h = "0123456789abcdef";
{T}j = 0;
{T}while (j < 16)
{T}{{
{T}{T}if (start + j < size)
{T}{T}{{
{T}{T}{T}write(1, &h[p[start + j] / 16], 1);
{T}{T}{T}write(1, &h[p[start + j] % 16], 1);
{T}{T}}}
{T}{T}else
{T}{T}{T}write(1, "  ", 2);
{T}{T}if (j % 2 == 1)
{T}{T}{T}write(1, " ", 1);
{T}{T}j++;
{T}}}
}}

static void{T}ft_ascii(unsigned char *p, unsigned int start, unsigned int size)
{{
{T}unsigned char{T}c;
{T}int{T}{T}{T}{T}j;

{T}j = 0;
{T}while (j < 16 && start + j < size)
{T}{{
{T}{T}c = p[start + j];
{T}{T}if (c >= 32 && c <= 126)
{T}{T}{T}write(1, &c, 1);
{T}{T}else
{T}{T}{T}write(1, ".", 1);
{T}{T}j++;
{T}}}
{T}write(1, "\\n", 1);
}}

void{T}*ft_print_memory(void *addr, unsigned int size)
{{
{T}unsigned char{T}*p;
{T}unsigned int{T}start;

{T}if (size == 0)
{T}{T}return (addr);
{T}p = (unsigned char *)addr;
{T}start = 0;
{T}while (start < size)
{T}{{
{T}{T}ft_addr((unsigned long)addr + start);
{T}{T}ft_hexline(p, start, size);
{T}{T}ft_ascii(p, start, size);
{T}{T}start = start + 16;
{T}}}
{T}return (addr);
}}
"""

# ============================== C 03 ==============================

F["c03/ex00/ft_strcmp.c"] = f"""int{T}ft_strcmp(char *s1, char *s2)
{{
{T}int{T}i;

{T}i = 0;
{T}while (s1[i] && s1[i] == s2[i])
{T}{T}i++;
{T}return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}}
"""

F["c03/ex01/ft_strncmp.c"] = f"""int{T}ft_strncmp(char *s1, char *s2, unsigned int n)
{{
{T}unsigned int{T}i;

{T}if (n == 0)
{T}{T}return (0);
{T}i = 0;
{T}while (i < n - 1 && s1[i] && s1[i] == s2[i])
{T}{T}i++;
{T}return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}}
"""

F["c03/ex02/ft_strcat.c"] = f"""char{T}*ft_strcat(char *dest, char *src)
{{
{T}int{T}i;
{T}int{T}j;

{T}i = 0;
{T}while (dest[i])
{T}{T}i++;
{T}j = 0;
{T}while (src[j])
{T}{{
{T}{T}dest[i + j] = src[j];
{T}{T}j++;
{T}}}
{T}dest[i + j] = '\\0';
{T}return (dest);
}}
"""

F["c03/ex03/ft_strncat.c"] = f"""char{T}*ft_strncat(char *dest, char *src, unsigned int nb)
{{
{T}unsigned int{T}i;
{T}unsigned int{T}j;

{T}i = 0;
{T}while (dest[i])
{T}{T}i++;
{T}j = 0;
{T}while (j < nb && src[j])
{T}{{
{T}{T}dest[i + j] = src[j];
{T}{T}j++;
{T}}}
{T}dest[i + j] = '\\0';
{T}return (dest);
}}
"""

F["c03/ex04/ft_strstr.c"] = f"""char{T}*ft_strstr(char *str, char *to_find)
{{
{T}int{T}i;
{T}int{T}j;

{T}if (!to_find[0])
{T}{T}return (str);
{T}i = 0;
{T}while (str[i])
{T}{{
{T}{T}j = 0;
{T}{T}while (str[i + j] && str[i + j] == to_find[j])
{T}{T}{T}j++;
{T}{T}if (!to_find[j])
{T}{T}{T}return (str + i);
{T}{T}i++;
{T}}}
{T}return (0);
}}
"""

F["c03/ex05/ft_strlcat.c"] = f"""unsigned int{T}ft_strlcat(char *dest, char *src, unsigned int size)
{{
{T}unsigned int{T}dlen;
{T}unsigned int{T}slen;
{T}unsigned int{T}i;

{T}dlen = 0;
{T}while (dest[dlen] && dlen < size)
{T}{T}dlen++;
{T}slen = 0;
{T}while (src[slen])
{T}{T}slen++;
{T}if (dlen == size)
{T}{T}return (size + slen);
{T}i = 0;
{T}while (src[i] && dlen + i + 1 < size)
{T}{{
{T}{T}dest[dlen + i] = src[i];
{T}{T}i++;
{T}}}
{T}dest[dlen + i] = '\\0';
{T}return (dlen + slen);
}}
"""

# ============================== C 04 ==============================

F["c04/ex00/ft_strlen.c"] = f"""int{T}ft_strlen(char *str)
{{
{T}int{T}i;

{T}i = 0;
{T}while (str[i])
{T}{T}i++;
{T}return (i);
}}
"""

F["c04/ex01/ft_putstr.c"] = f"""#include <unistd.h>

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

F["c04/ex02/ft_putnbr.c"] = f"""#include <unistd.h>

void{T}ft_putnbr(int nb)
{{
{T}long{T}n;
{T}char{T}c;

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

F["c04/ex03/ft_atoi.c"] = f"""int{T}ft_atoi(char *str)
{{
{T}int{T}i;
{T}int{T}sign;
{T}int{T}result;

{T}i = 0;
{T}sign = 1;
{T}result = 0;
{T}while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
{T}{T}i++;
{T}while (str[i] == '+' || str[i] == '-')
{T}{{
{T}{T}if (str[i] == '-')
{T}{T}{T}sign = -sign;
{T}{T}i++;
{T}}}
{T}while (str[i] >= '0' && str[i] <= '9')
{T}{{
{T}{T}result = result * 10 + (str[i] - '0');
{T}{T}i++;
{T}}}
{T}return (result * sign);
}}
"""

F["c04/ex04/ft_putnbr_base.c"] = f"""#include <unistd.h>

static int{T}ft_base_ok(char *base)
{{
{T}int{T}i;
{T}int{T}j;

{T}i = 0;
{T}while (base[i])
{T}{{
{T}{T}if (base[i] == '+' || base[i] == '-')
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
{T}if (i < 2)
{T}{T}return (0);
{T}return (1);
}}

static int{T}ft_baselen(char *base)
{{
{T}int{T}i;

{T}i = 0;
{T}while (base[i])
{T}{T}i++;
{T}return (i);
}}

static void{T}ft_put_base(long n, char *base, int len)
{{
{T}if (n >= len)
{T}{T}ft_put_base(n / len, base, len);
{T}write(1, &base[n % len], 1);
}}

void{T}ft_putnbr_base(int nbr, char *base)
{{
{T}long{T}n;
{T}int{T}{T}len;

{T}if (!ft_base_ok(base))
{T}{T}return ;
{T}len = ft_baselen(base);
{T}n = nbr;
{T}if (n < 0)
{T}{{
{T}{T}write(1, "-", 1);
{T}{T}n = -n;
{T}}}
{T}ft_put_base(n, base, len);
}}
"""

F["c04/ex05/ft_atoi_base.c"] = f"""static int{T}ft_base_ok(char *base)
{{
{T}int{T}i;
{T}int{T}j;

{T}i = 0;
{T}while (base[i])
{T}{{
{T}{T}if (base[i] == '+' || base[i] == '-')
{T}{T}{T}return (0);
{T}{T}if (base[i] == ' ' || (base[i] >= 9 && base[i] <= 13))
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
{T}if (i < 2)
{T}{T}return (0);
{T}return (1);
}}

static int{T}ft_baselen(char *base)
{{
{T}int{T}i;

{T}i = 0;
{T}while (base[i])
{T}{T}i++;
{T}return (i);
}}

static int{T}ft_index(char c, char *base)
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

static int{T}ft_convert(char *str, char *base, int len)
{{
{T}int{T}i;
{T}int{T}sign;
{T}int{T}result;

{T}i = 0;
{T}sign = 1;
{T}result = 0;
{T}while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
{T}{T}i++;
{T}while (str[i] == '+' || str[i] == '-')
{T}{{
{T}{T}if (str[i] == '-')
{T}{T}{T}sign = -sign;
{T}{T}i++;
{T}}}
{T}while (ft_index(str[i], base) >= 0)
{T}{{
{T}{T}result = result * len + ft_index(str[i], base);
{T}{T}i++;
{T}}}
{T}return (result * sign);
}}

int{T}ft_atoi_base(char *str, char *base)
{{
{T}if (!ft_base_ok(base))
{T}{T}return (0);
{T}return (ft_convert(str, base, ft_baselen(base)));
}}
"""

for rel, body in F.items():
    write_file(ROOT, rel, body)
print(f"\n{len(F)} archivos generados")
