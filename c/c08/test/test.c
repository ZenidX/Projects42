/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para los ejercicios de C08                      */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*                                                                            */
/*   Los ejercicios de C08 son HEADERS y MACROS (ft.h, ft_boolean.h,          */
/*   ft_abs.h, ft_point.h, ft_stock_str.h): por cada caso se escribe un       */
/*   main de test que incluye el header con #include y se compila con -I al   */
/*   directorio del ejercicio (-Wall -Wextra -Werror). Los casos marcados     */
/*   header_only no tienen fuente .c: srcs apunta al .h solo para el chequeo  */
/*   de existencia y se compila unicamente el main. Para ex04/ex05 se         */
/*   compilan ademas ft_strs_to_tab.c y ft_show_tab.c. Cada binario se        */
/*   ejecuta con fork+execv (con los argumentos del caso) y se comparan el    */
/*   stdout capturado byte a byte y el codigo de salida con lo esperado.      */
/*   Los ejercicios cuyo fichero no exista se marcan SKIP, no fallan.         */
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
	const char	*ex;          /* directorio del ejercicio (para mensajes)    */
	const char	*desc;
	const char	*srcs;        /* fuentes relativas al repo, sep. por espacio;
	                             si header_only, el .h (solo para access())  */
	const char	*inc;         /* dir para -I relativo al repo, o NULL        */
	int			header_only;  /* 1 = no pasar srcs a cc, compilar solo main  */
	const char	*a1;          /* argv[1] del programa de test, o NULL        */
	const char	*a2;          /* argv[2] del programa de test, o NULL        */
	int			want_code;    /* codigo de salida esperado                   */
	const char	*main_src;    /* contenido del main.c de test                */
	const char	*expected;    /* stdout esperado (byte a byte)               */
}	t_case;

static t_case	g_cases[] = {
	/* ex00: ft.h debe declarar los cinco prototipos. Se usa el main del     */
	/* script de referencia con definiciones de apoyo: si algun prototipo    */
	/* del header no coincide, la compilacion falla con -Werror.             */
	{"ex00", "ft.h (prototipos)", "ex00/ft.h", "ex00", 1, NULL, NULL, 2,
		"#include \"ft.h\"\n"
		"#include <unistd.h>\n"
		"void ft_putchar(char c){write(1,&c,1);}\n"
		"void ft_swap(int *a, int *b){int t=*a;*a=*b;*b=t;}\n"
		"void ft_putstr(char *str){while(*str)write(1,str++,1);}\n"
		"int ft_strlen(char *str){int n=0;while(str[n])n++;return n;}\n"
		"int ft_strcmp(char *s1, char *s2)"
		"{while(*s1&&*s1==*s2){s1++;s2++;}return *s1-*s2;}\n"
		"int main(void){int a=1,b=2;ft_swap(&a,&b);ft_putchar('B');"
		"ft_putstr(\"x\");\n"
		"return ft_strlen(\"hi\")+ft_strcmp(\"a\",\"a\");}\n",
		"Bx"},

	/* ex01: ft_boolean.h con el main del enunciado (par/impar segun argc)  */
	{"ex01", "boolean (0 args, par)", "ex01/ft_boolean.h", "ex01", 1,
		NULL, NULL, 0,
		"#include <unistd.h>\n"
		"#include \"ft_boolean.h\"\n"
		"\n"
		"void ft_putstr(char *str)\n"
		"{\n"
		"\twhile (*str)\n"
		"\t\twrite(1, str++, 1);\n"
		"}\n"
		"\n"
		"t_bool ft_is_even(int nbr)\n"
		"{\n"
		"\treturn ((EVEN(nbr)) ? TRUE : FALSE);\n"
		"}\n"
		"\n"
		"int main(int argc, char **argv)\n"
		"{\n"
		"\t(void)argv;\n"
		"\tif (ft_is_even(argc - 1) == TRUE)\n"
		"\t\tft_putstr(EVEN_MSG);\n"
		"\telse\n"
		"\t\tft_putstr(ODD_MSG);\n"
		"\treturn (SUCCESS);\n"
		"}\n",
		"I have an even number of arguments.\n"},

	{"ex01", "boolean (1 arg, impar)", "ex01/ft_boolean.h", "ex01", 1,
		"x", NULL, 0,
		"#include <unistd.h>\n"
		"#include \"ft_boolean.h\"\n"
		"\n"
		"void ft_putstr(char *str)\n"
		"{\n"
		"\twhile (*str)\n"
		"\t\twrite(1, str++, 1);\n"
		"}\n"
		"\n"
		"t_bool ft_is_even(int nbr)\n"
		"{\n"
		"\treturn ((EVEN(nbr)) ? TRUE : FALSE);\n"
		"}\n"
		"\n"
		"int main(int argc, char **argv)\n"
		"{\n"
		"\t(void)argv;\n"
		"\tif (ft_is_even(argc - 1) == TRUE)\n"
		"\t\tft_putstr(EVEN_MSG);\n"
		"\telse\n"
		"\t\tft_putstr(ODD_MSG);\n"
		"\treturn (SUCCESS);\n"
		"}\n",
		"I have an odd number of arguments.\n"},

	{"ex01", "boolean (2 args, par)", "ex01/ft_boolean.h", "ex01", 1,
		"x", "y", 0,
		"#include <unistd.h>\n"
		"#include \"ft_boolean.h\"\n"
		"\n"
		"void ft_putstr(char *str)\n"
		"{\n"
		"\twhile (*str)\n"
		"\t\twrite(1, str++, 1);\n"
		"}\n"
		"\n"
		"t_bool ft_is_even(int nbr)\n"
		"{\n"
		"\treturn ((EVEN(nbr)) ? TRUE : FALSE);\n"
		"}\n"
		"\n"
		"int main(int argc, char **argv)\n"
		"{\n"
		"\t(void)argv;\n"
		"\tif (ft_is_even(argc - 1) == TRUE)\n"
		"\t\tft_putstr(EVEN_MSG);\n"
		"\telse\n"
		"\t\tft_putstr(ODD_MSG);\n"
		"\treturn (SUCCESS);\n"
		"}\n",
		"I have an even number of arguments.\n"},

	/* ex02: macro ABS                                                      */
	{"ex02", "macro ABS", "ex02/ft_abs.h", "ex02", 1, NULL, NULL, 0,
		"#include \"ft_abs.h\"\n"
		"#include <stdio.h>\n"
		"int main(void){printf(\"%d %d %d\", ABS(-5), ABS(5), ABS(0)); "
		"return 0;}\n",
		"5 5 0"},

	/* ex03: struct t_point con el main del enunciado (verifica exit code)  */
	{"ex03", "ft_point.h (struct)", "ex03/ft_point.h", "ex03", 1,
		NULL, NULL, 0,
		"#include \"ft_point.h\"\n"
		"void set_point(t_point *point){point->x = 42; point->y = 21;}\n"
		"int main(void){t_point point; set_point(&point);\n"
		"return (point.x == 42 && point.y == 21) ? 0 : 1;}\n",
		""},

	/* ex04+ex05: ft_strs_to_tab alimenta ft_show_tab                       */
	{"ex04", "strs_to_tab + show_tab",
		"ex04/ft_strs_to_tab.c ex05/ft_show_tab.c", "ex04", 0,
		NULL, NULL, 0,
		"#include \"ft_stock_str.h\"\n"
		"struct s_stock_str *ft_strs_to_tab(int ac, char **av);\n"
		"void ft_show_tab(struct s_stock_str *par);\n"
		"int main(void){char *av[3] = {\"hola\", \"42\", \"mundo\"};\n"
		"ft_show_tab(ft_strs_to_tab(3, av)); return 0;}\n",
		"hola\n4\nhola\n42\n2\n42\nmundo\n5\nmundo\n"},
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

/* Primera fuente del caso (para access()); si header_only, es el .h. */
static void	first_src(char *dst, size_t n, const char *repo, const char *srcs)
{
	size_t	tl;

	tl = strcspn(srcs, " ");
	xsnprintf(dst, n, "%s/%.*s", repo, (int)tl, srcs);
}

/* Compila fuentes + main del caso. 0 ok, 1 error de compilación.
   Con header_only solo se compila el main (el header entra via -I). */
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
	if (c->header_only)
		srcs[0] = '\0';
	else
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
	strcpy(g_tmp, "/tmp/c08test.XXXXXX");
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
		char			*out;
		char			*pargv[4];
		int				sig;
		int				code;
		int				j;

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
		j = 0;
		pargv[j++] = bin;
		if (c->a1)
			pargv[j++] = (char *)c->a1;
		if (c->a2)
			pargv[j++] = (char *)c->a2;
		pargv[j] = NULL;
		code = run_bin(bin, pargv, &out, &sig);
		if (!sig && code == c->want_code && out
			&& strcmp(out, c->expected) == 0)
		{
			ok++;
			printf("%s[OK]%s   %s  %s\n", C_OK, C_0, c->ex, c->desc);
		}
		else
		{
			ko++;
			printf("%s[KO]%s   %s  %s\n", C_KO, C_0, c->ex, c->desc);
			printf("       esperado: \"");
			print_escaped(c->expected);
			printf("\"\n       obtenido: \"");
			print_escaped(out);
			printf("\"\n");
			if (sig)
				printf("       %smuere con señal %d (%s)%s\n", C_KO, sig,
					strsignal(sig), C_0);
			else if (code != c->want_code)
				printf("       %scódigo de salida %d (esperado %d)%s\n",
					C_D, code, c->want_code, C_0);
		}
		free(out);
	}
	printf("\n%s%d OK%s  %s%d KO%s  %s%d SKIP%s\n",
		C_OK, ok, C_0, C_KO, ko, C_0, C_SK, skipped, C_0);
	cleanup();
	return (ko != 0);
}
