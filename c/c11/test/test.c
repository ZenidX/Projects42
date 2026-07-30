/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para los ejercicios de C11 (punteros a func.)   */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*                                                                            */
/*   La mayoria de ejercicios de C11 son FUNCIONES: por cada caso se escribe  */
/*   un main de test en un directorio temporal, se compila junto a la fuente  */
/*   del repo con -Wall -Wextra -Werror, se ejecuta con fork+execv y se       */
/*   compara el stdout capturado byte a byte con lo esperado.                 */
/*   ex05 (do-op) es un PROGRAMA: main_src == NULL indica que no se escribe   */
/*   main.c (se compilan solo las fuentes, una unica vez) y el campo argv     */
/*   da los argumentos con los que ejecutar el binario en cada caso.          */
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
	const char	*main_src;  /* main.c de test; NULL = caso "programa"       */
	const char	*expected;  /* stdout esperado; NULL = usar gen()           */
	char		*(*gen)(void);
	char		**argv;     /* argv del programa (con argv[0], fin NULL)    */
}	t_case;

/* Fuentes del programa do-op (se compila una sola vez para todos sus casos) */
#define DOP_SRCS "ex05/do_op.c ex05/ops.c ex05/ft_atoi.c ex05/ft_putnbr.c"

/* Juegos de argumentos para do-op (argv[0] incluido; el nombre no importa,  */
/* execv usa la ruta real del binario).                                      */
static char	*g_dop_sum[] = {"do-op", "1", "+", "1", NULL};
static char	*g_dop_dirty[] = {"do-op", "42amis", "-", "--+-20toto12", NULL};
static char	*g_dop_unk[] = {"do-op", "1", "p", "1", NULL};
static char	*g_dop_toto3[] = {"do-op", "1", "+", "toto3", NULL};
static char	*g_dop_toto4[] = {"do-op", "toto3", "+", "4", NULL};
static char	*g_dop_plus[] = {"do-op", "foo", "plus", "bar", NULL};
static char	*g_dop_div0[] = {"do-op", "25", "/", "0", NULL};
static char	*g_dop_mod0[] = {"do-op", "25", "%", "0", NULL};
static char	*g_dop_div[] = {"do-op", "9", "/", "2", NULL};
static char	*g_dop_mod[] = {"do-op", "9", "%", "2", NULL};
static char	*g_dop_mul[] = {"do-op", "42", "*", "1", NULL};
static char	*g_dop_noargs[] = {"do-op", NULL};

static t_case	g_cases[] = {
	{"ex00", "ft_foreach", "ex00/ft_foreach.c", NULL,
		"#include <stdio.h>\n"
		"void ft_foreach(int *tab, int length, void (*f)(int));\n"
		"void pr(int n){ printf(\"%d,\", n); }\n"
		"int main(void){ int a[3]={1,2,3}; ft_foreach(a,3,&pr); "
		"ft_foreach(a,0,&pr); return 0; }\n",
		"1,2,3,", NULL, NULL},

	{"ex01", "ft_map", "ex01/ft_map.c", NULL,
		"#include <stdio.h>\n"
		"#include <stdlib.h>\n"
		"int *ft_map(int *tab, int length, int (*f)(int));\n"
		"int sq(int n){ return n*n; }\n"
		"int main(void){ int a[4]={1,2,3,4}; int *r=ft_map(a,4,&sq); int i=0;\n"
		"while(i<4){printf(\"%d,\",r[i]);i++;} free(r); return 0; }\n",
		"1,4,9,16,", NULL, NULL},

	{"ex02", "ft_any", "ex02/ft_any.c", NULL,
		"#include <stdio.h>\n"
		"int ft_any(char **tab, int (*f)(char*));\n"
		"int starta(char *s){ return s[0]==97; }\n"
		"int main(void){ char *a[]={\"xy\",\"ab\",0}; "
		"char *b[]={\"xy\",\"zz\",0};\n"
		"printf(\"%d%d\", ft_any(a,&starta), ft_any(b,&starta)); "
		"return 0; }\n",
		"10", NULL, NULL},

	{"ex03", "ft_count_if", "ex03/ft_count_if.c", NULL,
		"#include <stdio.h>\n"
		"int ft_count_if(char **tab, int length, int (*f)(char*));\n"
		"int starta(char *s){ return s[0]==97; }\n"
		"int main(void){ char *a[]={\"ab\",\"ax\",\"zz\",\"aa\"};\n"
		"printf(\"%d\", ft_count_if(a,4,&starta)); return 0; }\n",
		"3", NULL, NULL},

	{"ex04", "ft_is_sort", "ex04/ft_is_sort.c", NULL,
		"#include <stdio.h>\n"
		"int ft_is_sort(int *tab, int length, int (*f)(int,int));\n"
		"int cmp(int a,int b){ return a-b; }\n"
		"int main(void){ int a[]={1,2,3}; int b[]={3,2,1}; int c[]={1,3,2};\n"
		"printf(\"%d%d%d\", ft_is_sort(a,3,&cmp), ft_is_sort(b,3,&cmp), "
		"ft_is_sort(c,3,&cmp));\n"
		"return 0; }\n",
		"110", NULL, NULL},

	/* ex05 do-op: programa con varias fuentes, un binario y varios argv.    */
	/* La salida real lleva '\n' final (el shell lo recorta con $(...)).     */
	{"ex05", "do-op 1 + 1", DOP_SRCS, "ex05", NULL,
		"2\n", NULL, g_dop_sum},
	{"ex05", "do-op suma sucia", DOP_SRCS, "ex05", NULL,
		"62\n", NULL, g_dop_dirty},
	{"ex05", "do-op op desconocido p", DOP_SRCS, "ex05", NULL,
		"0\n", NULL, g_dop_unk},
	{"ex05", "do-op 1 + toto3", DOP_SRCS, "ex05", NULL,
		"1\n", NULL, g_dop_toto3},
	{"ex05", "do-op toto3 + 4", DOP_SRCS, "ex05", NULL,
		"4\n", NULL, g_dop_toto4},
	{"ex05", "do-op foo plus bar", DOP_SRCS, "ex05", NULL,
		"0\n", NULL, g_dop_plus},
	{"ex05", "do-op division por cero", DOP_SRCS, "ex05", NULL,
		"Stop : division by zero\n", NULL, g_dop_div0},
	{"ex05", "do-op modulo por cero", DOP_SRCS, "ex05", NULL,
		"Stop : modulo by zero\n", NULL, g_dop_mod0},
	{"ex05", "do-op 9 / 2", DOP_SRCS, "ex05", NULL,
		"4\n", NULL, g_dop_div},
	{"ex05", "do-op 9 % 2", DOP_SRCS, "ex05", NULL,
		"1\n", NULL, g_dop_mod},
	{"ex05", "do-op 42 * 1 ('*' literal)", DOP_SRCS, "ex05", NULL,
		"42\n", NULL, g_dop_mul},
	{"ex05", "do-op sin args", DOP_SRCS, "ex05", NULL,
		"", NULL, g_dop_noargs},

	{"ex06", "ft_sort_string_tab", "ex06/ft_sort_string_tab.c", NULL,
		"#include <stdio.h>\n"
		"void ft_sort_string_tab(char **tab);\n"
		"int main(void){ char *t[]={\"banana\",\"apple\",\"cherry\",0}; "
		"ft_sort_string_tab(t);\n"
		"int i=0; while(t[i]){printf(\"%s,\",t[i]);i++;} return 0; }\n",
		"apple,banana,cherry,", NULL, NULL},

	{"ex07", "advanced_sort (asc)", "ex07/ft_advanced_sort_string_tab.c", NULL,
		"#include <stdio.h>\n"
		"void ft_advanced_sort_string_tab(char **tab, int(*cmp)"
		"(char*,char*));\n"
		"int c(char*a,char*b){int i=0;while(a[i]&&a[i]==b[i])i++;"
		"return a[i]-b[i];}\n"
		"int main(void){ char *t[]={\"banana\",\"apple\",\"cherry\",0};\n"
		"ft_advanced_sort_string_tab(t,&c); int i=0; "
		"while(t[i]){printf(\"%s,\",t[i]);i++;}\n"
		"return 0; }\n",
		"apple,banana,cherry,", NULL, NULL},

	{"ex07", "advanced_sort (desc)", "ex07/ft_advanced_sort_string_tab.c",
		NULL,
		"#include <stdio.h>\n"
		"void ft_advanced_sort_string_tab(char **tab, int(*cmp)"
		"(char*,char*));\n"
		"int c(char*a,char*b){int i=0;while(a[i]&&a[i]==b[i])i++;"
		"return b[i]-a[i];}\n"
		"int main(void){ char *t[]={\"banana\",\"apple\",\"cherry\",0};\n"
		"ft_advanced_sort_string_tab(t,&c); int i=0; "
		"while(t[i]){printf(\"%s,\",t[i]);i++;}\n"
		"return 0; }\n",
		"cherry,banana,apple,", NULL, NULL},
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

/* Compila fuentes (+ main del caso si lo hay). 0 ok, 1 error de compilación.
   Si main_src es NULL (caso "programa") no se escribe main.c y se compilan
   solo las fuentes del repo. */
static int	compile_case(const char *repo, const t_case *c, const char *bin)
{
	char	srcs[PATHSZ * 2];
	char	inc[PATHSZ];
	char	cmd[PATHSZ * 4];
	char	mainp[PATHSZ];
	char	mainq[PATHSZ];
	FILE	*f;

	mainq[0] = '\0';
	if (c->main_src)
	{
		xsnprintf(mainp, sizeof(mainp), "%s/main.c", g_tmp);
		f = fopen(mainp, "w");
		if (!f)
			return (perror("fopen"), 1);
		fputs(c->main_src, f);
		fclose(f);
		xsnprintf(mainq, sizeof(mainq), "'%s'", mainp);
	}
	build_srcs(srcs, sizeof(srcs), repo, c->srcs);
	inc[0] = '\0';
	if (c->inc)
		xsnprintf(inc, sizeof(inc), "-I '%s/%s'", repo, c->inc);
	xsnprintf(cmd, sizeof(cmd),
		"cc -Wall -Wextra -Werror %s %s %s -o '%s' > '%s/cc.log' 2>&1",
		inc, srcs, mainq, bin, g_tmp);
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
	const char	*prog_srcs;   /* fuentes del ultimo programa compilado    */
	int			prog_rc;      /* resultado de esa compilacion             */
	int			ok;
	int			ko;
	int			skipped;
	size_t		i;

	repo = (argc > 1) ? argv[1] : detect_repo();
	init_colors();
	strcpy(g_tmp, "/tmp/c11test.XXXXXX");
	if (!mkdtemp(g_tmp))
		return (perror("mkdtemp"), 1);
	printf("repo: %s\n\n", repo);
	prog_srcs = NULL;
	prog_rc = 0;
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
		int				rc;

		first_src(src0, sizeof(src0), repo, c->srcs);
		if (access(src0, R_OK) != 0)
		{
			skipped++;
			printf("%s[SKIP]%s %s  %s%s%s\n", C_SK, C_0, c->ex, C_D,
				"fuente no encontrado", C_0);
			continue ;
		}
		/* Los casos "programa" comparten binario: se compila una vez y se
		   reutiliza mientras las fuentes no cambien. */
		if (!c->main_src)
		{
			xsnprintf(bin, sizeof(bin), "%s/progx", g_tmp);
			if (prog_srcs && strcmp(prog_srcs, c->srcs) == 0)
				rc = prog_rc;
			else
			{
				rc = compile_case(repo, c, bin);
				prog_srcs = c->srcs;
				prog_rc = rc;
			}
		}
		else
		{
			xsnprintf(bin, sizeof(bin), "%s/prog", g_tmp);
			rc = compile_case(repo, c, bin);
		}
		if (rc)
		{
			ko++;
			printf("%s[KO]%s   %s  %s %s(no compila)%s\n", C_KO, C_0,
				c->ex, c->desc, C_KO, C_0);
			dump_log();
			continue ;
		}
		pargv[0] = bin;
		pargv[1] = NULL;
		code = run_bin(bin, c->argv ? c->argv : pargv, &out, &sig);
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
