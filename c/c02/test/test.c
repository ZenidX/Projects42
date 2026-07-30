/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para los ejercicios de C02                      */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*                                                                            */
/*   Los ejercicios de C02 son FUNCIONES, no programas: por cada caso se      */
/*   escribe un main de test en un directorio temporal, se compila junto a    */
/*   la fuente del repo con -Wall -Wextra -Werror, se ejecuta con fork+execv  */
/*   y se compara el stdout capturado byte a byte con lo esperado.            */
/*   Los ejercicios cuyo .c no exista se marcan SKIP, no fallan.              */
/*                                                                            */
/*   Para ex12 (ft_print_memory) la dirección al inicio de cada línea es      */
/*   impredecible: el flag strip_addr elimina el prefijo "<hex>: " de cada    */
/*   línea del stdout capturado antes de comparar.                            */
/*                                                                            */
/* ************************************************************************** */

#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define PATHSZ 4096
#define TIMEOUT 5

/* ------------------------------- tabla de casos -------------------------- */

typedef struct s_case
{
	const char	*ex;        /* directorio del ejercicio (para mensajes)     */
	const char	*desc;
	const char	*srcs;      /* fuentes relativas al repo, sep. por espacio  */
	const char	*inc;       /* dir para -I relativo al repo, o NULL         */
	const char	*main_src;  /* contenido del main.c de test                 */
	const char	*expected;  /* stdout esperado; NULL = usar gen()           */
	char		*(*gen)(void);
	int			strip_addr; /* 1 = quitar prefijo "<hex>: " de cada línea   */
}	t_case;

/* Salida esperada de ex12: réplica del modelo de referencia (líneas de 16
   bytes, hex por pares con espacio cada 2 bytes, relleno con espacios y
   parte ascii con '.' para los no imprimibles), sin el prefijo de
   dirección, que se elimina también de la salida capturada. */

static char	*gen_memory(void)
{
	const unsigned char	data[] = "Bonjour a todos! 42\trules\n";
	const char			*hexd = "0123456789abcdef";
	size_t				len;
	char				*b;
	size_t				l;
	size_t				i;
	size_t				j;

	len = sizeof(data) - 1;
	b = malloc(1024);
	if (!b)
		return (NULL);
	l = 0;
	i = 0;
	while (i < len)
	{
		for (j = 0; j < 16; j++)
		{
			if (i + j < len)
			{
				b[l++] = hexd[data[i + j] / 16];
				b[l++] = hexd[data[i + j] % 16];
			}
			else
			{
				b[l++] = ' ';
				b[l++] = ' ';
			}
			if (j % 2 == 1)
				b[l++] = ' ';
		}
		for (j = 0; j < 16 && i + j < len; j++)
		{
			if (data[i + j] >= 32 && data[i + j] <= 126)
				b[l++] = (char)data[i + j];
			else
				b[l++] = '.';
		}
		b[l++] = '\n';
		i += 16;
	}
	b[l] = '\0';
	return (b);
}

static t_case	g_cases[] = {
	{"ex00", "strcpy (normal y vacia)", "ex00/ft_strcpy.c", NULL,
		"#include <stdio.h>\n"
		"char *ft_strcpy(char *d, char *s);\n"
		"int main(void)\n"
		"{\n"
		"\tchar\tdest[32];\n"
		"\n"
		"\tft_strcpy(dest, \"42 Barcelona\");\n"
		"\tprintf(\"%s|\", dest);\n"
		"\tprintf(\"%s\", ft_strcpy(dest, \"\"));\n"
		"\treturn (0);\n"
		"}\n",
		"42 Barcelona|", NULL, 0},

	{"ex01", "strncpy (relleno con '\\0')", "ex01/ft_strncpy.c", NULL,
		"#include <stdio.h>\n"
		"char *ft_strncpy(char *d, char *s, unsigned int n);\n"
		"int main(void)\n"
		"{\n"
		"\tchar\tdest[10];\n"
		"\tint\t\ti;\n"
		"\n"
		"\ti = 0;\n"
		"\twhile (i < 10)\n"
		"\t{\n"
		"\t\tdest[i] = 'X';\n"
		"\t\ti++;\n"
		"\t}\n"
		"\tft_strncpy(dest, \"ab\", 5);\n"
		"\ti = 0;\n"
		"\twhile (i < 10)\n"
		"\t{\n"
		"\t\tprintf(\"%c\", dest[i] ? dest[i] : '.');\n"
		"\t\ti++;\n"
		"\t}\n"
		"\treturn (0);\n"
		"}\n",
		"ab...XXXXX", NULL, 0},

	{"ex02", "str_is_alpha", "ex02/ft_str_is_alpha.c", NULL,
		"#include <stdio.h>\n"
		"int ft_str_is_alpha(char *s);\n"
		"int main(void)\n"
		"{\n"
		"\tprintf(\"%d%d%d%d\", ft_str_is_alpha(\"abcZ\"), "
		"ft_str_is_alpha(\"ab2\"),\n"
		"\t\tft_str_is_alpha(\"\"), ft_str_is_alpha(\"a b\"));\n"
		"\treturn (0);\n"
		"}\n",
		"1010", NULL, 0},

	{"ex03", "str_is_numeric", "ex03/ft_str_is_numeric.c", NULL,
		"#include <stdio.h>\n"
		"int ft_str_is_numeric(char *s);\n"
		"int main(void)\n"
		"{\n"
		"\tprintf(\"%d%d%d%d\", ft_str_is_numeric(\"0129\"), "
		"ft_str_is_numeric(\"12a\"),\n"
		"\t\tft_str_is_numeric(\"\"), ft_str_is_numeric(\"1 2\"));\n"
		"\treturn (0);\n"
		"}\n",
		"1010", NULL, 0},

	{"ex04", "str_is_lowercase", "ex04/ft_str_is_lowercase.c", NULL,
		"#include <stdio.h>\n"
		"int ft_str_is_lowercase(char *s);\n"
		"int main(void)\n"
		"{\n"
		"\tprintf(\"%d%d%d%d\", ft_str_is_lowercase(\"abc\"), "
		"ft_str_is_lowercase(\"aBc\"),\n"
		"\t\tft_str_is_lowercase(\"\"), ft_str_is_lowercase(\"a1\"));\n"
		"\treturn (0);\n"
		"}\n",
		"1010", NULL, 0},

	{"ex05", "str_is_uppercase", "ex05/ft_str_is_uppercase.c", NULL,
		"#include <stdio.h>\n"
		"int ft_str_is_uppercase(char *s);\n"
		"int main(void)\n"
		"{\n"
		"\tprintf(\"%d%d%d%d\", ft_str_is_uppercase(\"ABC\"), "
		"ft_str_is_uppercase(\"AbC\"),\n"
		"\t\tft_str_is_uppercase(\"\"), ft_str_is_uppercase(\"A1\"));\n"
		"\treturn (0);\n"
		"}\n",
		"1010", NULL, 0},

	{"ex06", "str_is_printable", "ex06/ft_str_is_printable.c", NULL,
		"#include <stdio.h>\n"
		"int ft_str_is_printable(char *s);\n"
		"int main(void)\n"
		"{\n"
		"\tprintf(\"%d%d%d\", ft_str_is_printable(\"Hola 42!\"),\n"
		"\t\tft_str_is_printable(\"a\\tb\"), ft_str_is_printable(\"\"));\n"
		"\treturn (0);\n"
		"}\n",
		"101", NULL, 0},

	{"ex07", "strupcase", "ex07/ft_strupcase.c", NULL,
		"#include <stdio.h>\n"
		"char *ft_strupcase(char *s);\n"
		"int main(void)\n"
		"{\n"
		"\tchar\ts[] = \"Hola 42 mundo!\";\n"
		"\n"
		"\tprintf(\"%s\", ft_strupcase(s));\n"
		"\treturn (0);\n"
		"}\n",
		"HOLA 42 MUNDO!", NULL, 0},

	{"ex08", "strlowcase", "ex08/ft_strlowcase.c", NULL,
		"#include <stdio.h>\n"
		"char *ft_strlowcase(char *s);\n"
		"int main(void)\n"
		"{\n"
		"\tchar\ts[] = \"Hola 42 MUNDO!\";\n"
		"\n"
		"\tprintf(\"%s\", ft_strlowcase(s));\n"
		"\treturn (0);\n"
		"}\n",
		"hola 42 mundo!", NULL, 0},

	{"ex09", "strcapitalize", "ex09/ft_strcapitalize.c", NULL,
		"#include <stdio.h>\n"
		"char *ft_strcapitalize(char *s);\n"
		"int main(void)\n"
		"{\n"
		"\tchar\ts[] = \"salut, comment tu vas ? 42mots quarante-deux; "
		"cinquante+et+un\";\n"
		"\n"
		"\tprintf(\"%s\", ft_strcapitalize(s));\n"
		"\treturn (0);\n"
		"}\n",
		"Salut, Comment Tu Vas ? 42mots Quarante-Deux; Cinquante+Et+Un",
		NULL, 0},

	{"ex10", "strlcpy (truncado y entero)", "ex10/ft_strlcpy.c", NULL,
		"#include <stdio.h>\n"
		"unsigned int ft_strlcpy(char *d, char *s, unsigned int size);\n"
		"int main(void)\n"
		"{\n"
		"\tchar\t\t\tdest[16];\n"
		"\tunsigned int\tr;\n"
		"\n"
		"\tr = ft_strlcpy(dest, \"Hello\", 3);\n"
		"\tprintf(\"%s %u|\", dest, r);\n"
		"\tr = ft_strlcpy(dest, \"Hi\", 16);\n"
		"\tprintf(\"%s %u\", dest, r);\n"
		"\treturn (0);\n"
		"}\n",
		"He 5|Hi 2", NULL, 0},

	{"ex11", "putstr_non_printable", "ex11/ft_putstr_non_printable.c", NULL,
		"void ft_putstr_non_printable(char *s);\n"
		"int main(void)\n"
		"{\n"
		"\tft_putstr_non_printable(\"Coucou\\ntu vas bien ?\");\n"
		"\treturn (0);\n"
		"}\n",
		"Coucou\\0atu vas bien ?", NULL, 0},

	{"ex12", "print_memory (sin direccion)", "ex12/ft_print_memory.c", NULL,
		"#include <unistd.h>\n"
		"void *ft_print_memory(void *addr, unsigned int size);\n"
		"int main(void)\n"
		"{\n"
		"\tchar\t*s;\n"
		"\n"
		"\ts = \"Bonjour a todos! 42\\trules\\n\";\n"
		"\tft_print_memory(s, 26);\n"
		"\treturn (0);\n"
		"}\n",
		NULL, gen_memory, 1},
};
#define N_CASES (sizeof(g_cases) / sizeof(g_cases[0]))

/* --------------------------------- utilidades ---------------------------- */

static const char	*C_OK = "";
static const char	*C_KO = "";
static const char	*C_SK = "";
static const char	*C_D  = "";
static const char	*C_0  = "";

static void	init_colors(void)
{
	if (!isatty(STDOUT_FILENO))
		return ;
	C_OK = "\033[32m";
	C_KO = "\033[31m";
	C_SK = "\033[33m";
	C_D  = "\033[90m";
	C_0  = "\033[0m";
}

static void	xsnprintf(char *dst, size_t n, const char *fmt, ...)
{
	va_list	ap;
	int		r;

	va_start(ap, fmt);
	r = vsnprintf(dst, n, fmt, ap);
	va_end(ap);
	if (r < 0 || (size_t)r >= n)
	{
		fprintf(stderr, "error: ruta demasiado larga\n");
		exit(1);
	}
}

static void	print_escaped(const char *s)
{
	if (!s)
	{
		printf("(null)");
		return ;
	}
	while (*s)
	{
		if (*s == '\n')
			printf("\\n");
		else if (*s == '\t')
			printf("\\t");
		else if ((unsigned char)*s < 32 || (unsigned char)*s == 127)
			printf("\\x%02x", (unsigned char)*s);
		else
			putchar(*s);
		s++;
	}
}

static char	*read_all(int fd)
{
	size_t	cap;
	size_t	len;
	ssize_t	n;
	char	*buf;
	char	*tmp;

	cap = 4096;
	len = 0;
	buf = malloc(cap);
	if (!buf)
		return (NULL);
	while ((n = read(fd, buf + len, cap - len - 1)) > 0)
	{
		len += (size_t)n;
		if (len + 1 >= cap)
		{
			cap *= 2;
			tmp = realloc(buf, cap);
			if (!tmp)
				return (free(buf), NULL);
			buf = tmp;
		}
	}
	buf[len] = '\0';
	return (buf);
}

/* Elimina in situ el prefijo "<hex>: " (una o mas cifras hex seguidas de
   ": ") del inicio de cada línea del buffer. */

static int	is_hex_digit(char c)
{
	return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')
		|| (c >= 'A' && c <= 'F'));
}

static void	strip_addr_prefix(char *s)
{
	char	*r;
	char	*w;
	char	*p;

	r = s;
	w = s;
	while (*r)
	{
		p = r;
		while (is_hex_digit(*p))
			p++;
		if (p > r && p[0] == ':' && p[1] == ' ')
			r = p + 2;
		while (*r && *r != '\n')
			*w++ = *r++;
		if (*r == '\n')
			*w++ = *r++;
	}
	*w = '\0';
}

/* -------------------------------- compilación ---------------------------- */

static char	g_tmp[64];

/* Copia en dst las fuentes del caso prefijadas con el repo y entre
   comillas: "'repo/a.c' 'repo/b.c'". */
static void	build_srcs(char *dst, size_t n, const char *repo, const char *srcs)
{
	size_t	l;
	size_t	tl;
	const char	*p;

	l = 0;
	p = srcs;
	dst[0] = '\0';
	while (*p)
	{
		tl = strcspn(p, " ");
		if (l + tl + strlen(repo) + 8 >= n)
		{
			fprintf(stderr, "error: lista de fuentes demasiado larga\n");
			exit(1);
		}
		l += (size_t)sprintf(dst + l, "%s'%s/%.*s'", l ? " " : "",
				repo, (int)tl, p);
		p += tl;
		while (*p == ' ')
			p++;
	}
}

/* Primera fuente del caso (para access()). */
static void	first_src(char *dst, size_t n, const char *repo, const char *srcs)
{
	size_t	tl;

	tl = strcspn(srcs, " ");
	xsnprintf(dst, n, "%s/%.*s", repo, (int)tl, srcs);
}

/* Compila fuentes + main del caso. 0 ok, 1 error de compilación. */
static int	compile_case(const char *repo, const t_case *c, const char *bin)
{
	char	srcs[PATHSZ * 2];
	char	inc[PATHSZ];
	char	cmd[PATHSZ * 4];
	char	mainp[PATHSZ];
	FILE	*f;

	xsnprintf(mainp, sizeof(mainp), "%s/main.c", g_tmp);
	f = fopen(mainp, "w");
	if (!f)
		return (perror("fopen"), 1);
	fputs(c->main_src, f);
	fclose(f);
	build_srcs(srcs, sizeof(srcs), repo, c->srcs);
	inc[0] = '\0';
	if (c->inc)
		xsnprintf(inc, sizeof(inc), "-I '%s/%s'", repo, c->inc);
	xsnprintf(cmd, sizeof(cmd),
		"cc -Wall -Wextra -Werror %s %s '%s' -o '%s' > '%s/cc.log' 2>&1",
		inc, srcs, mainp, bin, g_tmp);
	return (system(cmd) != 0);
}

static void	dump_log(void)
{
	char	path[PATHSZ];
	char	*log;
	int		fd;

	xsnprintf(path, sizeof(path), "%s/cc.log", g_tmp);
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return ;
	log = read_all(fd);
	close(fd);
	if (log && *log)
		printf("%s%s%s", C_D, log, C_0);
	free(log);
}

/* --------------------------------- ejecución ----------------------------- */

static int	run_bin(const char *bin, char **argv, char **out, int *sig)
{
	int		fds[2];
	int		status;
	pid_t	pid;

	*out = NULL;
	*sig = 0;
	if (pipe(fds) == -1)
		return (-1);
	pid = fork();
	if (pid == -1)
		return (close(fds[0]), close(fds[1]), -1);
	if (pid == 0)
	{
		close(fds[0]);
		dup2(fds[1], STDOUT_FILENO);
		close(fds[1]);
		alarm(TIMEOUT);
		execv(bin, argv);
		_exit(127);
	}
	close(fds[1]);
	*out = read_all(fds[0]);
	close(fds[0]);
	if (waitpid(pid, &status, 0) == -1)
		return (-1);
	if (WIFSIGNALED(status))
		*sig = WTERMSIG(status);
	return (WIFEXITED(status) ? WEXITSTATUS(status) : -1);
}

/* ----------------------------------- main -------------------------------- */

static void	cleanup(void)
{
	char	cmd[PATHSZ + 16];

	if (g_tmp[0])
	{
		xsnprintf(cmd, sizeof(cmd), "rm -rf '%s'", g_tmp);
		if (system(cmd) != 0)
			fprintf(stderr, "aviso: no se pudo limpiar %s\n", g_tmp);
	}
}

static const char	*detect_repo(void)
{
	if (access("repo/ex00", F_OK) == 0)
		return ("repo");
	if (access("ex00", F_OK) == 0)
		return (".");
	if (access("../repo/ex00", F_OK) == 0)
		return ("../repo");
	return ("..");
}

int	main(int argc, char **argv)
{
	const char	*repo;
	int			ok;
	int			ko;
	int			skipped;
	size_t		i;

	repo = (argc > 1) ? argv[1] : detect_repo();
	init_colors();
	strcpy(g_tmp, "/tmp/c02test.XXXXXX");
	if (!mkdtemp(g_tmp))
		return (perror("mkdtemp"), 1);
	printf("repo: %s\n\n", repo);
	ok = 0;
	ko = 0;
	skipped = 0;
	i = 0;
	while (i < N_CASES)
	{
		const t_case	*c = &g_cases[i++];
		char			src0[PATHSZ];
		char			bin[PATHSZ];
		char			*exp;
		char			*out;
		char			*pargv[2];
		int				sig;
		int				code;

		first_src(src0, sizeof(src0), repo, c->srcs);
		if (access(src0, R_OK) != 0)
		{
			skipped++;
			printf("%s[SKIP]%s %s  %s%s%s\n", C_SK, C_0, c->ex, C_D,
				"fuente no encontrado", C_0);
			continue ;
		}
		xsnprintf(bin, sizeof(bin), "%s/prog", g_tmp);
		if (compile_case(repo, c, bin))
		{
			ko++;
			printf("%s[KO]%s   %s  %s %s(no compila)%s\n", C_KO, C_0,
				c->ex, c->desc, C_KO, C_0);
			dump_log();
			continue ;
		}
		pargv[0] = bin;
		pargv[1] = NULL;
		code = run_bin(bin, pargv, &out, &sig);
		if (c->strip_addr && out)
			strip_addr_prefix(out);
		exp = c->gen ? c->gen() : NULL;
		if (!sig && out && strcmp(out, c->gen ? exp : c->expected) == 0)
		{
			ok++;
			printf("%s[OK]%s   %s  %s\n", C_OK, C_0, c->ex, c->desc);
		}
		else
		{
			ko++;
			printf("%s[KO]%s   %s  %s\n", C_KO, C_0, c->ex, c->desc);
			printf("       esperado: \"");
			print_escaped(c->gen ? exp : c->expected);
			printf("\"\n       obtenido: \"");
			print_escaped(out);
			printf("\"\n");
			if (sig)
				printf("       %smuere con señal %d (%s)%s\n", C_KO, sig,
					strsignal(sig), C_0);
			else if (code != 0)
				printf("       %ssalida con código %d%s\n", C_D, code, C_0);
		}
		free(exp);
		free(out);
	}
	printf("\n%s%d OK%s  %s%d KO%s  %s%d SKIP%s\n",
		C_OK, ok, C_0, C_KO, ko, C_0, C_SK, skipped, C_0);
	cleanup();
	return (ko != 0);
}
