/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para los ejercicios de C01                      */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*                                                                            */
/*   Los ejercicios de C01 son FUNCIONES, no programas: por cada caso se      */
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

static t_case	g_cases[] = {
	{"ex00", "ft_ft pone 42", "ex00/ft_ft.c", NULL,
		"#include <stdio.h>\n"
		"void ft_ft(int *n);\n"
		"int main(void){int x=0;ft_ft(&x);printf(\"%d\",x);return 0;}\n",
		"42", NULL},

	{"ex01", "ultimate_ft (9 niveles)", "ex01/ft_ultimate_ft.c", NULL,
		"#include <stdio.h>\n"
		"void ft_ultimate_ft(int *********n);\n"
		"int main(void){int x=0;int *p1=&x;int **p2=&p1;int ***p3=&p2;"
		"int ****p4=&p3;int *****p5=&p4;\n"
		"int ******p6=&p5;int *******p7=&p6;int ********p8=&p7;"
		"int *********p9=&p8;\n"
		"ft_ultimate_ft(p9);printf(\"%d\",x);return 0;}\n",
		"42", NULL},

	{"ex02", "swap de dos ints", "ex02/ft_swap.c", NULL,
		"#include <stdio.h>\n"
		"void ft_swap(int *a, int *b);\n"
		"int main(void){int a=3,b=9;ft_swap(&a,&b);"
		"printf(\"%d %d\",a,b);return 0;}\n",
		"9 3", NULL},

	{"ex03", "div_mod(17,5)", "ex03/ft_div_mod.c", NULL,
		"#include <stdio.h>\n"
		"void ft_div_mod(int a, int b, int *d, int *m);\n"
		"int main(void){int d,m;ft_div_mod(17,5,&d,&m);"
		"printf(\"%d %d\",d,m);return 0;}\n",
		"3 2", NULL},

	{"ex04", "ultimate_div_mod(17,5)", "ex04/ft_ultimate_div_mod.c", NULL,
		"#include <stdio.h>\n"
		"void ft_ultimate_div_mod(int *a, int *b);\n"
		"int main(void){int a=17,b=5;ft_ultimate_div_mod(&a,&b);"
		"printf(\"%d %d\",a,b);return 0;}\n",
		"3 2", NULL},

	{"ex05", "putstr", "ex05/ft_putstr.c", NULL,
		"void ft_putstr(char *s);\n"
		"int main(void){ft_putstr(\"Hola 42\");return 0;}\n",
		"Hola 42", NULL},

	{"ex06", "strlen (vacia y normal)", "ex06/ft_strlen.c", NULL,
		"#include <stdio.h>\n"
		"int ft_strlen(char *s);\n"
		"int main(void){printf(\"%d %d\",ft_strlen(\"\"),"
		"ft_strlen(\"hola mundo\"));return 0;}\n",
		"0 10", NULL},

	{"ex07", "rev_int_tab (impar y par)", "ex07/ft_rev_int_tab.c", NULL,
		"#include <stdio.h>\n"
		"void ft_rev_int_tab(int *t, int s);\n"
		"int main(void)\n"
		"{\n"
		"\tint\tt[5] = {1, 2, 3, 4, 5};\n"
		"\tint\tu[4] = {9, 8, 7, 6};\n"
		"\tint\ti;\n"
		"\n"
		"\tft_rev_int_tab(t, 5);\n"
		"\tft_rev_int_tab(u, 4);\n"
		"\ti = 0;\n"
		"\twhile (i < 5)\n"
		"\t\tprintf(\"%d\", t[i++]);\n"
		"\tprintf(\" \");\n"
		"\ti = 0;\n"
		"\twhile (i < 4)\n"
		"\t\tprintf(\"%d\", u[i++]);\n"
		"\treturn (0);\n"
		"}\n",
		"54321 6789", NULL},

	{"ex08", "sort_int_tab (con duplicados)", "ex08/ft_sort_int_tab.c", NULL,
		"#include <stdio.h>\n"
		"void ft_sort_int_tab(int *t, int s);\n"
		"int main(void){int t[7]={5,-2,9,0,5,-8,3};int i;\n"
		"ft_sort_int_tab(t,7);for(i=0;i<7;i++)printf(\"%d \",t[i]);"
		"return 0;}\n",
		"-8 -2 0 3 5 5 9 ", NULL},
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
	strcpy(g_tmp, "/tmp/c01test.XXXXXX");
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
