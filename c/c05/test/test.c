/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para los ejercicios de C05                      */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*                                                                            */
/*   Los ejercicios de C05 son FUNCIONES, no programas: por cada caso se      */
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
	{"ex00", "factorial iterativo (0,1,5,4,-3)",
		"ex00/ft_iterative_factorial.c", NULL,
		"#include <stdio.h>\n"
		"int ft_iterative_factorial(int n);\n"
		"int main(void){printf(\"%d %d %d %d %d\",ft_iterative_factorial(0),\n"
		"ft_iterative_factorial(1),ft_iterative_factorial(5),\n"
		"ft_iterative_factorial(4),ft_iterative_factorial(-3));return 0;}\n",
		"1 1 120 24 0", NULL},

	{"ex01", "factorial recursivo (0,1,5,4,-3)",
		"ex01/ft_recursive_factorial.c", NULL,
		"#include <stdio.h>\n"
		"int ft_recursive_factorial(int n);\n"
		"int main(void){printf(\"%d %d %d %d %d\",ft_recursive_factorial(0),\n"
		"ft_recursive_factorial(1),ft_recursive_factorial(5),\n"
		"ft_recursive_factorial(4),ft_recursive_factorial(-3));return 0;}\n",
		"1 1 120 24 0", NULL},

	{"ex02", "potencia iterativa (2^10,5^0,0^0,2^3,3^-2)",
		"ex02/ft_iterative_power.c", NULL,
		"#include <stdio.h>\n"
		"int ft_iterative_power(int n,int p);\n"
		"int main(void){printf(\"%d %d %d %d %d\",ft_iterative_power(2,10),\n"
		"ft_iterative_power(5,0),ft_iterative_power(0,0),\n"
		"ft_iterative_power(2,3),ft_iterative_power(3,-2));return 0;}\n",
		"1024 1 1 8 0", NULL},

	{"ex03", "potencia recursiva (2^10,5^0,0^0,2^3,3^-2)",
		"ex03/ft_recursive_power.c", NULL,
		"#include <stdio.h>\n"
		"int ft_recursive_power(int n,int p);\n"
		"int main(void){printf(\"%d %d %d %d %d\",ft_recursive_power(2,10),\n"
		"ft_recursive_power(5,0),ft_recursive_power(0,0),\n"
		"ft_recursive_power(2,3),ft_recursive_power(3,-2));return 0;}\n",
		"1024 1 1 8 0", NULL},

	{"ex04", "fibonacci (0,1,2,9,-1,10)",
		"ex04/ft_fibonacci.c", NULL,
		"#include <stdio.h>\n"
		"int ft_fibonacci(int i);\n"
		"int main(void){printf(\"%d %d %d %d %d %d\",ft_fibonacci(0),"
		"ft_fibonacci(1),\n"
		"ft_fibonacci(2),ft_fibonacci(9),ft_fibonacci(-1),ft_fibonacci(10));"
		"return 0;}\n",
		"0 1 1 34 -1 55", NULL},

	{"ex05", "raiz cuadrada (0,1,4,2,25,2147395600)",
		"ex05/ft_sqrt.c", NULL,
		"#include <stdio.h>\n"
		"int ft_sqrt(int n);\n"
		"int main(void){printf(\"%d %d %d %d %d %d\",ft_sqrt(0),ft_sqrt(1),"
		"ft_sqrt(4),\n"
		"ft_sqrt(2),ft_sqrt(25),ft_sqrt(2147395600));return 0;}\n",
		"0 1 2 0 5 46340", NULL},

	{"ex06", "es primo (0,1,2,7,9,-5,7919)",
		"ex06/ft_is_prime.c", NULL,
		"#include <stdio.h>\n"
		"int ft_is_prime(int n);\n"
		"int main(void){printf(\"%d%d%d%d%d%d%d\",ft_is_prime(0),ft_is_prime(1),\n"
		"ft_is_prime(2),ft_is_prime(7),ft_is_prime(9),ft_is_prime(-5),\n"
		"ft_is_prime(7919));return 0;}\n",
		"0011001", NULL},

	{"ex07", "siguiente primo (0,14,17,20,-5)",
		"ex07/ft_find_next_prime.c", NULL,
		"#include <stdio.h>\n"
		"int ft_find_next_prime(int n);\n"
		"int main(void){printf(\"%d %d %d %d %d\",ft_find_next_prime(0),\n"
		"ft_find_next_prime(14),ft_find_next_prime(17),\n"
		"ft_find_next_prime(20),ft_find_next_prime(-5));return 0;}\n",
		"2 17 17 23 2", NULL},

	/* El main captura el stdout de la funcion en un tmpfile, cuenta las      */
	/* lineas de 10 digitos, comprueba la primera solucion y el retorno, y    */
	/* imprime "retorno lineas primera_ok" para comparar con lo esperado.     */
	{"ex08", "ten queens (724 sols, 1a=0257948136)",
		"ex08/ft_ten_queens_puzzle.c", NULL,
		"#include <stdio.h>\n"
		"#include <string.h>\n"
		"#include <unistd.h>\n"
		"int ft_ten_queens_puzzle(void);\n"
		"int main(void)\n"
		"{\n"
		"\tFILE *t = tmpfile();\n"
		"\tchar buf[64];\n"
		"\tint n, saved, lines = 0, okf = 0;\n"
		"\tif (!t)\n"
		"\t\treturn 1;\n"
		"\tfflush(stdout);\n"
		"\tsaved = dup(1);\n"
		"\tdup2(fileno(t), 1);\n"
		"\tn = ft_ten_queens_puzzle();\n"
		"\tfflush(stdout);\n"
		"\tdup2(saved, 1);\n"
		"\tclose(saved);\n"
		"\trewind(t);\n"
		"\twhile (fgets(buf, sizeof(buf), t))\n"
		"\t\tif (strlen(buf) == 11 && buf[10] == '\\n'\n"
		"\t\t\t&& strspn(buf, \"0123456789\") == 10)\n"
		"\t\t{\n"
		"\t\t\tif (lines == 0)\n"
		"\t\t\t\tokf = (strncmp(buf, \"0257948136\", 10) == 0);\n"
		"\t\t\tlines++;\n"
		"\t\t}\n"
		"\tfclose(t);\n"
		"\tprintf(\"%d %d %d\", n, lines, okf);\n"
		"\treturn 0;\n"
		"}\n",
		"724 724 1", NULL},
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
	strcpy(g_tmp, "/tmp/c05test.XXXXXX");
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
