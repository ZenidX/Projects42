/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para los ejercicios de C00                      */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*                                                                            */
/*   Los ejercicios de C00 son FUNCIONES, no programas: por cada caso se      */
/*   escribe un main de test en un directorio temporal, se compila junto a    */
/*   la fuente del repo con -Wall -Wextra -Werror, se ejecuta con fork+execv  */
/*   y se compara el stdout capturado byte a byte con lo esperado.            */
/*   Los ejercicios cuyo .c no exista se marcan SKIP, no fallan.              */
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
}	t_case;

/* Salidas largas (combinaciones): se generan aqui mismo en vez de           */
/* incrustar constantes de 40 KB en el fichero.                              */

static char	*gen_comb(void)
{
	char	*b;
	size_t	l;
	int		i;
	int		j;
	int		k;

	b = malloc(8192);
	if (!b)
		return (NULL);
	l = 0;
	for (i = 0; i <= 7; i++)
		for (j = i + 1; j <= 8; j++)
			for (k = j + 1; k <= 9; k++)
			{
				if (l)
				{
					b[l++] = ',';
					b[l++] = ' ';
				}
				b[l++] = (char)('0' + i);
				b[l++] = (char)('0' + j);
				b[l++] = (char)('0' + k);
			}
	b[l] = '\0';
	return (b);
}

static char	*gen_comb2(void)
{
	char	*b;
	size_t	l;
	int		i;
	int		j;

	b = malloc(65536);
	if (!b)
		return (NULL);
	l = 0;
	for (i = 0; i <= 98; i++)
		for (j = i + 1; j <= 99; j++)
			l += (size_t)sprintf(b + l, "%s%02d %02d", l ? ", " : "", i, j);
	return (b);
}

static char	*gen_combn1(void)
{
	char	*b;
	size_t	l;
	int		i;

	b = malloc(64);
	if (!b)
		return (NULL);
	l = 0;
	for (i = 0; i <= 9; i++)
		l += (size_t)sprintf(b + l, "%s%d", l ? ", " : "", i);
	return (b);
}

static t_case	g_cases[] = {
	{"ex00", "ft_putchar('B')", "ex00/ft_putchar.c", NULL,
		"void ft_putchar(char c);\n"
		"int main(void){ft_putchar(0x42);return 0;}\n",
		"B", NULL},

	{"ex01", "alfabeto", "ex01/ft_print_alphabet.c", NULL,
		"void ft_print_alphabet(void);\n"
		"int main(void){ft_print_alphabet();return 0;}\n",
		"abcdefghijklmnopqrstuvwxyz", NULL},

	{"ex02", "alfabeto invertido", "ex02/ft_print_reverse_alphabet.c", NULL,
		"void ft_print_reverse_alphabet(void);\n"
		"int main(void){ft_print_reverse_alphabet();return 0;}\n",
		"zyxwvutsrqponmlkjihgfedcba", NULL},

	{"ex03", "digitos 0-9", "ex03/ft_print_numbers.c", NULL,
		"void ft_print_numbers(void);\n"
		"int main(void){ft_print_numbers();return 0;}\n",
		"0123456789", NULL},

	{"ex04", "negativo/cero/positivo", "ex04/ft_is_negative.c", NULL,
		"void ft_is_negative(int n);\n"
		"int main(void){ft_is_negative(-5);ft_is_negative(0);"
		"ft_is_negative(7);return 0;}\n",
		"NPP", NULL},

	{"ex05", "print_comb", "ex05/ft_print_comb.c", NULL,
		"void ft_print_comb(void);\n"
		"int main(void){ft_print_comb();return 0;}\n",
		NULL, gen_comb},

	{"ex06", "print_comb2", "ex06/ft_print_comb2.c", NULL,
		"void ft_print_comb2(void);\n"
		"int main(void){ft_print_comb2();return 0;}\n",
		NULL, gen_comb2},

	{"ex07", "putnbr (limites int)", "ex07/ft_putnbr.c", NULL,
		"void ft_putnbr(int n);\n"
		"int main(void){ft_putnbr(42);ft_putnbr(0);ft_putnbr(-2147483648);"
		"ft_putnbr(2147483647);ft_putnbr(-7);return 0;}\n",
		"420-21474836482147483647-7", NULL},

	{"ex08", "print_combn(3)", "ex08/ft_print_combn.c", NULL,
		"void ft_print_combn(int n);\n"
		"int main(void){ft_print_combn(3);return 0;}\n",
		NULL, gen_comb},

	{"ex08", "print_combn(1)", "ex08/ft_print_combn.c", NULL,
		"void ft_print_combn(int n);\n"
		"int main(void){ft_print_combn(1);return 0;}\n",
		NULL, gen_combn1},
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
	strcpy(g_tmp, "/tmp/c00test.XXXXXX");
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
