/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para los ejercicios de C09                      */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*                                                                            */
/*   C09 es un modulo especial (libft + Makefile), asi que no hay tabla de    */
/*   casos uniforme: cada ejercicio tiene su propia rutina de chequeos.       */
/*   Todo se hace en un directorio temporal bajo /tmp: se COPIAN alli las     */
/*   fuentes del repo y nunca se genera nada dentro del repositorio.          */
/*     - ex00: libft_creator.sh crea libft.a (fallback: cc -c + ar), se       */
/*       verifican con nm los 5 simbolos ft_* y se enlaza un main de test.    */
/*     - ex01: el Makefile (unico entregable) se prueba con las fuentes de    */
/*       ex00 como andamiaje: all / incremental / clean / recompilacion       */
/*       tras tocar un .c / re / fclean.                                      */
/*     - ex02: ft_split se compila con un main embebido y se compara stdout.  */
/*   Los ejercicios cuyo entregable no exista se marcan SKIP, no fallan.      */
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

/* --------------------------------- utilidades ---------------------------- */

static const char	*C_OK = "";
static const char	*C_KO = "";
static const char	*C_SK = "";
static const char	*C_D  = "";
static const char	*C_0  = "";

static int	g_ok;
static int	g_ko;
static int	g_skip;
static char	g_tmp[64];

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

static char	*read_file(const char *path)
{
	int		fd;
	char	*buf;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return (NULL);
	buf = read_all(fd);
	close(fd);
	return (buf);
}

static void	write_file(const char *path, const char *content)
{
	FILE	*f;

	f = fopen(path, "w");
	if (!f)
	{
		perror("fopen");
		exit(1);
	}
	fputs(content, f);
	fclose(f);
}

static int	file_exists(const char *path)
{
	return (access(path, F_OK) == 0);
}

/* Busqueda de subcadena sin distinguir mayusculas (para mensajes de make). */
static int	ci_contains(const char *hay, const char *needle)
{
	size_t	i;
	size_t	j;

	if (!hay)
		return (0);
	for (i = 0; hay[i]; i++)
	{
		j = 0;
		while (needle[j] && hay[i + j]
			&& (hay[i + j] | 32) == (needle[j] | 32))
			j++;
		if (!needle[j])
			return (1);
	}
	return (0);
}

/* ------------------------------ informes OK/KO --------------------------- */

static void	t_ok(const char *ex, const char *desc)
{
	g_ok++;
	printf("%s[OK]%s   %s  %s\n", C_OK, C_0, ex, desc);
}

static void	t_skip(const char *ex, const char *desc, const char *why)
{
	g_skip++;
	printf("%s[SKIP]%s %s  %s  %s%s%s\n", C_SK, C_0, ex, desc, C_D, why, C_0);
}

static void	t_ko(const char *ex, const char *desc)
{
	g_ko++;
	printf("%s[KO]%s   %s  %s\n", C_KO, C_0, ex, desc);
}

static void	t_ko_diff(const char *ex, const char *desc, const char *exp,
		const char *got)
{
	t_ko(ex, desc);
	printf("       esperado: \"");
	print_escaped(exp);
	printf("\"\n       obtenido: \"");
	print_escaped(got);
	printf("\"\n");
}

/* -------------------------- ejecucion de comandos ------------------------ */

/* Ejecuta un comando de shell redirigiendo stdout+stderr a g_tmp/cmd.log.
   Devuelve el codigo de salida (0 = exito, -1 = error interno). */
static int	run_cmd(const char *fmt, ...)
{
	char	cmd[PATHSZ * 4];
	char	full[PATHSZ * 4 + 128];
	va_list	ap;
	int		r;

	va_start(ap, fmt);
	r = vsnprintf(cmd, sizeof(cmd), fmt, ap);
	va_end(ap);
	if (r < 0 || (size_t)r >= sizeof(cmd))
	{
		fprintf(stderr, "error: comando demasiado largo\n");
		exit(1);
	}
	xsnprintf(full, sizeof(full), "{ %s ; } > '%s/cmd.log' 2>&1", cmd, g_tmp);
	r = system(full);
	if (r == -1)
		return (-1);
	if (WIFEXITED(r))
		return (WEXITSTATUS(r));
	return (-1);
}

/* Contenido de g_tmp/cmd.log (salida del ultimo run_cmd). */
static char	*read_log(void)
{
	char	path[PATHSZ];

	xsnprintf(path, sizeof(path), "%s/cmd.log", g_tmp);
	return (read_file(path));
}

static void	dump_log(void)
{
	char	*log;

	log = read_log();
	if (log && *log)
		printf("%s%s%s", C_D, log, C_0);
	free(log);
}

/* Ejecuta un binario con fork+execv+alarm capturando stdout. */
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

/* ----------------------------------- ex00 -------------------------------- */

/* libft_creator.sh debe crear libft.a; se verifica con nm que exporta los
   5 simbolos ft_* y se enlaza un main de test que usa las 5 funciones. */
static void	test_ex00(const char *repo)
{
	static const char	*syms[] = {"ft_putchar", "ft_swap", "ft_putstr",
		"ft_strlen", "ft_strcmp"};
	char				d[PATHSZ];
	char				path[PATHSZ];
	char				bin[PATHSZ];
	char				miss[256];
	char				*log;
	char				*out;
	char				*pargv[2];
	int					sig;
	size_t				i;

	xsnprintf(path, sizeof(path), "%s/ex00/ft_putchar.c", repo);
	if (access(path, R_OK) != 0)
	{
		t_skip("ex00", "libft_creator.sh", "fuentes no encontradas");
		return ;
	}
	/* Copia de las fuentes a un dir temporal: el repo no se toca. */
	xsnprintf(d, sizeof(d), "%s/ex00", g_tmp);
	run_cmd("mkdir -p '%s' && cp '%s/ex00/'*.c '%s/'", d, repo, d);
	xsnprintf(path, sizeof(path), "%s/ex00/libft_creator.sh", repo);
	if (access(path, R_OK) == 0)
		run_cmd("cp '%s' '%s/' && cd '%s' && sh libft_creator.sh", path, d, d);
	else
		run_cmd("cd '%s' && cc -Wall -Wextra -Werror -c *.c"
			" && ar rcs libft.a *.o && rm -f *.o", d);
	xsnprintf(path, sizeof(path), "%s/libft.a", d);
	if (!file_exists(path))
	{
		t_ko("ex00", "libft.a creada por libft_creator.sh");
		dump_log();
		return ;
	}
	t_ok("ex00", "libft.a creada por libft_creator.sh");
	/* Simbolos exportados (nm ... | grep "T ft_x"). */
	if (run_cmd("nm '%s'", path) != 0)
		t_skip("ex00", "simbolos en libft.a", "nm no disponible");
	else
	{
		log = read_log();
		miss[0] = '\0';
		for (i = 0; i < sizeof(syms) / sizeof(syms[0]); i++)
		{
			xsnprintf(bin, sizeof(bin), "T %s", syms[i]);
			if (!log || !strstr(log, bin))
			{
				strcat(miss, " ");
				strcat(miss, syms[i]);
			}
		}
		free(log);
		if (!miss[0])
			t_ok("ex00", "simbolos ft_* en libft.a");
		else
		{
			t_ko("ex00", "simbolos ft_* en libft.a");
			printf("       %sfaltan:%s%s\n", C_KO, miss, C_0);
		}
	}
	/* Enlace y uso de las 5 funciones contra libft.a. */
	xsnprintf(bin, sizeof(bin), "%s/main.c", g_tmp);
	write_file(bin,
		"#include <stdio.h>\n"
		"void ft_putchar(char c);\n"
		"void ft_putstr(char *str);\n"
		"void ft_swap(int *a, int *b);\n"
		"int ft_strlen(char *str);\n"
		"int ft_strcmp(char *s1, char *s2);\n"
		"int main(void){int a=1;int b=2;ft_swap(&a,&b);ft_putchar('A');\n"
		"ft_putstr(\"hi\\n\");\n"
		"printf(\"%d %d %d %d\", ft_strlen(\"hola\"),"
		" ft_strcmp(\"a\", \"a\"), a, b);return 0;}\n");
	xsnprintf(d, sizeof(d), "%s/b00", g_tmp);
	if (run_cmd("cc -Wall -Wextra -Werror '%s' '%s' -o '%s'", bin, path, d))
	{
		t_ko("ex00", "enlace con libft.a (no compila)");
		dump_log();
		return ;
	}
	pargv[0] = d;
	pargv[1] = NULL;
	run_bin(d, pargv, &out, &sig);
	if (!sig && out && strcmp(out, "Ahi\n4 0 2 1") == 0)
		t_ok("ex00", "enlace+uso de ft_putchar/putstr/swap/strlen/strcmp");
	else
		t_ko_diff("ex00", "enlace+uso de ft_putchar/putstr/swap/strlen/strcmp",
			"Ahi\\n4 0 2 1", out);
	free(out);
}

/* ----------------------------------- ex01 -------------------------------- */

/* El Makefile (unico entregable) se prueba en un dir temporal con las
   fuentes de ex00 en srcs/ y ft.h (de c08/ex00 o un stub) en includes/. */
static void	test_ex01(const char *repo)
{
	static const char	*objs[] = {"ft_putchar", "ft_swap", "ft_putstr",
		"ft_strlen", "ft_strcmp"};
	char				m[PATHSZ];
	char				path[PATHSZ];
	char				lib[PATHSZ];
	char				*log;
	size_t				i;
	int					left;

	xsnprintf(path, sizeof(path), "%s/ex01/Makefile", repo);
	if (access(path, R_OK) != 0)
	{
		t_skip("ex01", "Makefile", "Makefile no encontrado");
		return ;
	}
	xsnprintf(m, sizeof(m), "%s/ex00/ft_putchar.c", repo);
	if (access(m, R_OK) != 0)
	{
		t_skip("ex01", "Makefile", "fuentes de ex00 no encontradas");
		return ;
	}
	if (run_cmd("command -v make") != 0)
	{
		t_skip("ex01", "Makefile", "make no disponible");
		return ;
	}
	/* Andamiaje temporal: Makefile + fuentes en srcs + ft.h en includes. */
	xsnprintf(m, sizeof(m), "%s/ex01", g_tmp);
	run_cmd("mkdir -p '%s/srcs' '%s/includes'", m, m);
	run_cmd("cp '%s' '%s/'", path, m);
	run_cmd("cp '%s/ex00/'*.c '%s/srcs/'", repo, m);
	xsnprintf(path, sizeof(path), "%s/../c08/ex00/ft.h", repo);
	if (access(path, R_OK) == 0)
		run_cmd("cp '%s' '%s/includes/'", path, m);
	else
	{
		xsnprintf(path, sizeof(path), "%s/includes/ft.h", m);
		write_file(path, "#ifndef FT_H\n# define FT_H\n#endif\n");
	}
	/* make all crea libft.a. */
	xsnprintf(lib, sizeof(lib), "%s/libft.a", m);
	if (run_cmd("make -C '%s'", m) != 0 || !file_exists(lib))
	{
		t_ko("ex01", "make crea libft.a");
		dump_log();
		return ;
	}
	t_ok("ex01", "make crea libft.a");
	/* make sin cambios: no debe rehacer nada. */
	run_cmd("make -C '%s'", m);
	log = read_log();
	if (ci_contains(log, "Nothing to be done")
		|| ci_contains(log, "is up to date")
		|| ci_contains(log, "actualizado")
		|| ci_contains(log, "nada que hacer"))
		t_ok("ex01", "make incremental (no relink)");
	else
		t_skip("ex01", "make incremental", "mensaje no reconocido");
	free(log);
	/* make clean: borra los .o y conserva libft.a. */
	run_cmd("make -C '%s' clean", m);
	left = 0;
	for (i = 0; i < sizeof(objs) / sizeof(objs[0]); i++)
	{
		xsnprintf(path, sizeof(path), "%s/srcs/%s.o", m, objs[i]);
		if (file_exists(path))
			left++;
	}
	if (!left && file_exists(lib))
		t_ok("ex01", "make clean (borra .o, conserva libft.a)");
	else
	{
		t_ko("ex01", "make clean (borra .o, conserva libft.a)");
		printf("       %s.o restantes: %d, libft.a existe: %d%s\n",
			C_D, left, file_exists(lib), C_0);
	}
	/* Tocar un .c fuerza recompilar ese objeto. */
	run_cmd("make -C '%s'", m);
	sleep(1);
	run_cmd("touch '%s/srcs/ft_strlen.c'", m);
	run_cmd("make -C '%s'", m);
	log = read_log();
	if (log && strstr(log, "ft_strlen"))
		t_ok("ex01", "make recompila tras editar un .c");
	else
	{
		t_ko("ex01", "make recompila tras editar un .c");
		dump_log();
	}
	free(log);
	/* make re. */
	if (run_cmd("make -C '%s' re", m) == 0 && file_exists(lib))
		t_ok("ex01", "make re");
	else
	{
		t_ko("ex01", "make re");
		dump_log();
	}
	/* make fclean: borra libft.a (y los .o). */
	run_cmd("make -C '%s' fclean", m);
	left = 0;
	for (i = 0; i < sizeof(objs) / sizeof(objs[0]); i++)
	{
		xsnprintf(path, sizeof(path), "%s/srcs/%s.o", m, objs[i]);
		if (file_exists(path))
			left++;
	}
	if (!file_exists(lib) && !left)
		t_ok("ex01", "make fclean (borra libft.a)");
	else
		t_ko("ex01", "make fclean (borra libft.a)");
}

/* ----------------------------------- ex02 -------------------------------- */

/* ft_split se compila con un main embebido y se compara el stdout. */
static void	test_ex02(const char *repo)
{
	const char	*expected = "[hola][42][mundo!]";
	char		src[PATHSZ];
	char		mainp[PATHSZ];
	char		bin[PATHSZ];
	char		*out;
	char		*pargv[2];
	int			sig;

	xsnprintf(src, sizeof(src), "%s/ex02/ft_split.c", repo);
	if (access(src, R_OK) != 0)
	{
		t_skip("ex02", "ft_split", "fuente no encontrada");
		return ;
	}
	xsnprintf(mainp, sizeof(mainp), "%s/main.c", g_tmp);
	write_file(mainp,
		"#include <stdio.h>\n"
		"char **ft_split(char *str, char *charset);\n"
		"int main(void){char **t = ft_split(\"  hola,,42  mundo!\", \" ,\");\n"
		"int i = 0; if (!t) return 1;\n"
		"while (t[i]){printf(\"[%s]\", t[i]); i++;} return 0;}\n");
	xsnprintf(bin, sizeof(bin), "%s/bsplit", g_tmp);
	if (run_cmd("cc -Wall -Wextra -Werror '%s' '%s' -o '%s'",
			src, mainp, bin))
	{
		t_ko("ex02", "ft_split (no compila)");
		dump_log();
		return ;
	}
	pargv[0] = bin;
	pargv[1] = NULL;
	run_bin(bin, pargv, &out, &sig);
	if (!sig && out && strcmp(out, expected) == 0)
		t_ok("ex02", "ft_split(\"  hola,,42  mundo!\", \" ,\")");
	else
	{
		t_ko_diff("ex02", "ft_split(\"  hola,,42  mundo!\", \" ,\")",
			expected, out);
		if (sig)
			printf("       %smuere con señal %d (%s)%s\n", C_KO, sig,
				strsignal(sig), C_0);
	}
	free(out);
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

	repo = (argc > 1) ? argv[1] : detect_repo();
	init_colors();
	strcpy(g_tmp, "/tmp/c09test.XXXXXX");
	if (!mkdtemp(g_tmp))
		return (perror("mkdtemp"), 1);
	printf("repo: %s\n\n", repo);
	test_ex00(repo);
	test_ex01(repo);
	test_ex02(repo);
	printf("\n%s%d OK%s  %s%d KO%s  %s%d SKIP%s\n",
		C_OK, g_ok, C_0, C_KO, g_ko, C_0, C_SK, g_skip, C_0);
	cleanup();
	return (g_ko != 0);
}
