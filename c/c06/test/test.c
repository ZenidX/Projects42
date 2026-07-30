/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para los ejercicios de C06                      */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*   Ejecutar:  ./test/runner [ruta_repo] [--extra]                           */
/*                                                                            */
/*   Por cada ejercicio del array g_ex:                                       */
/*     1. compila repo/exNN/fuente.c con -Wall -Wextra -Werror                */
/*     2. lanza el binario con fork+execv controlando argv[0]                 */
/*     3. captura stdout y lo compara con la salida esperada                  */
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

/* ----------------------------- tabla de ejercicios ----------------------- */

typedef struct s_ex
{
	const char	*dir;
	const char	*src;
	char		bin[PATHSZ];
	int			state;      /* 0 ok, 1 no compila, 2 no existe */
}	t_ex;

static t_ex	g_ex[] = {
	{"ex00", "ft_print_program_name.c", {0}, 2},
	{"ex01", "ft_print_params.c",       {0}, 2},
	{"ex02", "ft_rev_params.c",         {0}, 2},
	{"ex03", "ft_sort_params.c",        {0}, 2},
};
#define N_EX (sizeof(g_ex) / sizeof(g_ex[0]))

/* ------------------------------- tabla de casos -------------------------- */

typedef struct s_case
{
	const char	*ex;
	const char	*desc;
	char		**argv;       /* argv[0] incluido; NULL-terminado */
	const char	*expected;    /* stdout esperado; NULL = solo comprobar que no crashea */
	int			extra;        /* 1 = solo con --extra */
}	t_case;

static char	*av00_a[] = {"./ft_print_program_name", NULL};
static char	*av00_b[] = {"pouic", NULL};
static char	*av00_c[] = {"/usr/local/bin/ft_print_program_name", NULL};
static char	*av00_d[] = {"./a.out", "argumentos", "ignorados", NULL};
static char	*av00_e[] = {NULL};

static char	*av01_a[] = {"./ft_print_params", "hola", "que", "tal", NULL};
static char	*av01_b[] = {"./ft_print_params", NULL};
static char	*av01_c[] = {"./ft_print_params", "", "x", NULL};

static char	*av02_a[] = {"./ft_rev_params", "hola", "que", "tal", NULL};
static char	*av02_b[] = {"./ft_rev_params", NULL};
static char	*av02_c[] = {"./ft_rev_params", "solo", NULL};

static char	*av03_a[] = {"./ft_sort_params", "3", "1", "2", NULL};
static char	*av03_b[] = {"./ft_sort_params", "abc", "Abc", "0", "z", NULL};
static char	*av03_c[] = {"./ft_sort_params", "aa", "a", "aaa", NULL};
static char	*av03_d[] = {"./ft_sort_params", NULL};

static t_case	g_cases[] = {
	{"ex00", "nombre relativo",        av00_a, "./ft_print_program_name\n", 0},
	{"ex00", "nombre sin ./",          av00_b, "pouic\n", 0},
	{"ex00", "ruta absoluta",          av00_c, "/usr/local/bin/ft_print_program_name\n", 0},
	{"ex00", "ignora otros args",      av00_d, "./a.out\n", 0},
	{"ex00", "argc == 0 (solo no crash)", av00_e, NULL, 1},

	{"ex01", "tres params",            av01_a, "hola\nque\ntal\n", 0},
	{"ex01", "sin params",             av01_b, "", 0},
	{"ex01", "param vacio",            av01_c, "\nx\n", 0},

	{"ex02", "tres params al reves",   av02_a, "tal\nque\nhola\n", 0},
	{"ex02", "sin params",             av02_b, "", 0},
	{"ex02", "un solo param",          av02_c, "solo\n", 0},

	{"ex03", "numeros",                av03_a, "1\n2\n3\n", 0},
	{"ex03", "orden ASCII no alfabetico", av03_b, "0\nAbc\nabc\nz\n", 0},
	{"ex03", "prefijos",               av03_c, "a\naa\naaa\n", 0},
	{"ex03", "sin params",             av03_d, "", 0},
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

/* snprintf con control de truncado; evita ademas los avisos
   -Wformat-truncation al construir rutas. */
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

static int	compile_ex(const char *repo, t_ex *ex)
{
	char	src[PATHSZ];
	char	cmd[PATHSZ * 3];

	xsnprintf(src, sizeof(src), "%s/%s/%s", repo, ex->dir, ex->src);
	if (access(src, R_OK) != 0)
		return (ex->state = 2);
	xsnprintf(ex->bin, sizeof(ex->bin), "%s/%s", g_tmp, ex->dir);
	xsnprintf(cmd, sizeof(cmd),
		"cc -Wall -Wextra -Werror -o '%s' '%s' > '%s/%s.log' 2>&1",
		ex->bin, src, g_tmp, ex->dir);
	if (system(cmd) != 0)
		return (ex->state = 1);
	return (ex->state = 0);
}

static void	dump_log(const t_ex *ex)
{
	char	path[PATHSZ];
	char	*log;
	int		fd;

	xsnprintf(path, sizeof(path), "%s/%s.log", g_tmp, ex->dir);
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

/* Lanza bin con el argv indicado. Devuelve el exit code (o -1),
   escribe la salida en *out y la señal recibida en *sig. */
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

static t_ex	*find_ex(const char *dir)
{
	size_t	i;

	i = 0;
	while (i < N_EX)
	{
		if (strcmp(g_ex[i].dir, dir) == 0)
			return (&g_ex[i]);
		i++;
	}
	return (NULL);
}

static void	print_argv(char **argv)
{
	int	i;

	i = 0;
	printf("argv = [");
	while (argv[i])
	{
		printf("%s\"%s\"", i ? ", " : "", argv[i]);
		i++;
	}
	printf("]");
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

int	main(int argc, char **argv)
{
	const char	*repo;
	int			extra;
	int			ok;
	int			ko;
	int			skipped;
	size_t		i;

	repo = NULL;
	extra = 0;
	i = 1;
	while ((int)i < argc)
	{
		if (strcmp(argv[i], "--extra") == 0)
			extra = 1;
		else
			repo = argv[i];
		i++;
	}
	if (!repo)
	{
		if (access("repo/ex00", F_OK) == 0)
			repo = "repo";
		else if (access("ex00", F_OK) == 0)
			repo = ".";
		else if (access("../repo/ex00", F_OK) == 0)
			repo = "../repo";
		else
			repo = "..";
	}
	init_colors();
	strcpy(g_tmp, "/tmp/c06test.XXXXXX");
	if (!mkdtemp(g_tmp))
		return (perror("mkdtemp"), 1);
	printf("repo: %s\n\n", repo);

	/* fase 1: compilación */
	i = 0;
	while (i < N_EX)
	{
		compile_ex(repo, &g_ex[i]);
		if (g_ex[i].state == 1)
		{
			printf("%s[COMPILA KO]%s %s/%s\n", C_KO, C_0,
				g_ex[i].dir, g_ex[i].src);
			dump_log(&g_ex[i]);
		}
		i++;
	}

	/* fase 2: ejecución de casos */
	ok = 0;
	ko = 0;
	skipped = 0;
	i = 0;
	while (i < N_CASES)
	{
		t_case	*c = &g_cases[i++];
		t_ex	*ex = find_ex(c->ex);
		char	*out;
		int		sig;
		int		code;

		if (!ex || ex->state != 0 || (c->extra && !extra))
		{
			skipped++;
			printf("%s[SKIP]%s %s  %s%s%s\n", C_SK, C_0, c->ex, C_D,
				(ex && ex->state == 1) ? "no compila"
				: (c->extra && !extra) ? "test extra (usa --extra)"
				: "fuente no encontrado", C_0);
			continue ;
		}
		code = run_bin(ex->bin, c->argv, &out, &sig);
		if (!sig && (!c->expected || (out && strcmp(out, c->expected) == 0)))
		{
			ok++;
			printf("%s[OK]%s   %s  %s\n", C_OK, C_0, c->ex, c->desc);
		}
		else
		{
			ko++;
			printf("%s[KO]%s   %s  %s\n", C_KO, C_0, c->ex, c->desc);
			printf("       ");
			print_argv(c->argv);
			if (c->expected)
			{
				printf("\n       esperado: \"");
				print_escaped(c->expected);
				printf("\"\n       obtenido: \"");
				print_escaped(out);
				printf("\"");
			}
			printf("\n");
			if (sig)
				printf("       %smuere con señal %d (%s)%s\n", C_KO, sig,
					strsignal(sig), C_0);
			else if (code != 0)
				printf("       %ssalida con código %d%s\n", C_D, code, C_0);
		}
		free(out);
	}
	printf("\n%s%d OK%s  %s%d KO%s  %s%d SKIP%s\n",
		C_OK, ok, C_0, C_KO, ko, C_0, C_SK, skipped, C_0);
	cleanup();
	return (ko != 0);
}
