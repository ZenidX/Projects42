/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para los ejercicios de C10                      */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*                                                                            */
/*   Los ejercicios de C10 son PROGRAMAS completos: cada uno se compila       */
/*   directamente con cc -Wall -Wextra -Werror y TODAS sus fuentes hacia un   */
/*   directorio temporal (no se usan sus Makefiles para no dejar artefactos   */
/*   en el repo). Los ficheros de prueba tambien viven en el dir temporal.    */
/*   Cada programa se ejecuta con fork+execv+alarm capturando stdout y        */
/*   stderr por pipes separados y alimentando stdin via dup2 cuando toca.     */
/*   Las salidas esperadas se calculan EN C (leyendo el fichero de prueba y   */
/*   generando el formato de cat/tail/hexdump que valida el shell de          */
/*   referencia). Las comparaciones recortan los '\n' finales, igual que      */
/*   hace $(...) en el shell. El caso "fichero ilegible" usa una ruta         */
/*   inexistente (como el shell): chmod 000 no sirve si se ejecuta como root. */
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
#define DATASZ 65536

/* Contenidos de los ficheros de prueba (los mismos del shell). */
#define F1_CONTENT "Linea uno\nLinea dos\nfin sin salto"
#define F2_CONTENT "ABCDEFGHIJ"

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

/* Rutas de los ficheros de prueba (rellenadas en main). */
static char	g_f1[PATHSZ];
static char	g_f2[PATHSZ];
static char	g_empty[PATHSZ];
static char	g_stdin[PATHSZ];
static char	g_nope[PATHSZ];

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

/* Recorta los '\n' finales, igual que hace $(...) en el shell. */
static void	trim_nl(char *s)
{
	size_t	l;

	if (!s)
		return ;
	l = strlen(s);
	while (l > 0 && s[l - 1] == '\n')
		s[--l] = '\0';
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

/* Compara got (recortado) con expected (recortado); sig != 0 siempre es KO. */
static void	check_out(const char *ex, const char *desc, char *got,
		const char *expected, int sig)
{
	char	*exp;

	exp = strdup(expected ? expected : "");
	if (!exp)
	{
		perror("strdup");
		exit(1);
	}
	trim_nl(exp);
	trim_nl(got);
	if (!sig && got && strcmp(got, exp) == 0)
		t_ok(ex, desc);
	else
	{
		t_ko(ex, desc);
		printf("       esperado: \"");
		print_escaped(exp);
		printf("\"\n       obtenido: \"");
		print_escaped(got);
		printf("\"\n");
		if (sig)
			printf("       %smuere con señal %d (%s)%s\n", C_KO, sig,
				strsignal(sig), C_0);
	}
	free(exp);
}

/* -------------------------------- compilación ---------------------------- */

/* Copia en dst las fuentes prefijadas con el repo y entre comillas. */
static void	build_srcs(char *dst, size_t n, const char *repo, const char *srcs)
{
	size_t		l;
	size_t		tl;
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

/* Primera fuente (para access()). */
static void	first_src(char *dst, size_t n, const char *repo, const char *srcs)
{
	size_t	tl;

	tl = strcspn(srcs, " ");
	xsnprintf(dst, n, "%s/%.*s", repo, (int)tl, srcs);
}

/* Compila TODAS las fuentes del programa hacia un binario del tmp. */
static int	compile_prog(const char *repo, const char *srcs, const char *bin)
{
	char	s[PATHSZ * 2];
	char	cmd[PATHSZ * 4];

	build_srcs(s, sizeof(s), repo, srcs);
	xsnprintf(cmd, sizeof(cmd),
		"cc -Wall -Wextra -Werror %s -o '%s' > '%s/cc.log' 2>&1",
		s, bin, g_tmp);
	return (system(cmd) != 0);
}

static void	dump_log(void)
{
	char	path[PATHSZ];
	char	*log;

	xsnprintf(path, sizeof(path), "%s/cc.log", g_tmp);
	log = read_file(path);
	if (log && *log)
		printf("%s%s%s", C_D, log, C_0);
	free(log);
}

/* --------------------------------- ejecución ----------------------------- */

/* Ejecuta un binario con fork+execv+alarm capturando stdout y stderr por
   pipes separados; stdin_file (o /dev/null) se conecta a stdin con dup2.
   Devuelve el codigo de salida; sig recoge la señal si muere por señal. */
static int	run_prog(const char *bin, char **argv, const char *stdin_file,
		char **out, char **err, int *sig)
{
	int		ofds[2];
	int		efds[2];
	int		fd;
	int		status;
	pid_t	pid;

	*out = NULL;
	*err = NULL;
	*sig = 0;
	if (pipe(ofds) == -1)
		return (-1);
	if (pipe(efds) == -1)
		return (close(ofds[0]), close(ofds[1]), -1);
	pid = fork();
	if (pid == -1)
		return (close(ofds[0]), close(ofds[1]),
			close(efds[0]), close(efds[1]), -1);
	if (pid == 0)
	{
		fd = open(stdin_file ? stdin_file : "/dev/null", O_RDONLY);
		if (fd >= 0)
		{
			dup2(fd, STDIN_FILENO);
			close(fd);
		}
		dup2(ofds[1], STDOUT_FILENO);
		dup2(efds[1], STDERR_FILENO);
		close(ofds[0]);
		close(ofds[1]);
		close(efds[0]);
		close(efds[1]);
		alarm(TIMEOUT);
		execv(bin, argv);
		_exit(127);
	}
	close(ofds[1]);
	close(efds[1]);
	*out = read_all(ofds[0]);
	*err = read_all(efds[0]);
	close(ofds[0]);
	close(efds[0]);
	if (waitpid(pid, &status, 0) == -1)
		return (-1);
	if (WIFSIGNALED(status))
		*sig = WTERMSIG(status);
	return (WIFEXITED(status) ? WEXITSTATUS(status) : -1);
}

/* -------------------------- salidas esperadas en C ----------------------- */

/* Concatenacion de ficheros (referencia de cat). */
static char	*gen_cat(const char **files, int nfiles)
{
	char	*res;
	char	*c;
	size_t	l;
	int		i;

	res = malloc(DATASZ);
	if (!res)
		return (NULL);
	l = 0;
	for (i = 0; i < nfiles; i++)
	{
		c = read_file(files[i]);
		if (c)
		{
			l += (size_t)sprintf(res + l, "%s", c);
			free(c);
		}
	}
	res[l] = '\0';
	return (res);
}

/* Ultimos `count` bytes de cada fichero, con cabeceras "==> f <==" y linea
   en blanco entre ficheros cuando hay varios (formato de tail). */
static char	*gen_tail(const char **files, int nfiles, int count)
{
	char		*res;
	char		*c;
	const char	*start;
	size_t		l;
	size_t		len;
	int			i;

	res = malloc(DATASZ);
	if (!res)
		return (NULL);
	l = 0;
	for (i = 0; i < nfiles; i++)
	{
		c = read_file(files[i]);
		if (!c)
			continue ;
		len = strlen(c);
		start = c;
		if (len > (size_t)count)
			start = c + (len - (size_t)count);
		if (nfiles > 1)
			l += (size_t)sprintf(res + l, "%s==> %s <==\n",
					i ? "\n" : "", files[i]);
		l += (size_t)sprintf(res + l, "%s", start);
		free(c);
	}
	res[l] = '\0';
	return (res);
}

/* Referencia del hexdump clasico (formato por defecto, igual que el shell):
   lineas "offset(7hex) grupo grupo ..." con grupos de 2 bytes en orden
   little-endian (%02x%02x de b1,b0), rellenadas a 47 columnas, offset
   continuo entre ficheros y offset total final si hubo datos. */
static char	*gen_hexdump(const char **files, int nfiles)
{
	unsigned char	data[DATASZ];
	char			*res;
	size_t			len;
	size_t			off;
	size_t			ch;
	size_t			l;
	size_t			start;
	size_t			i;
	ssize_t			n;
	int				f;
	int				fd;
	unsigned int	b0;
	unsigned int	b1;

	len = 0;
	for (f = 0; f < nfiles; f++)
	{
		fd = open(files[f], O_RDONLY);
		if (fd < 0)
			continue ;
		while ((n = read(fd, data + len, DATASZ - len)) > 0)
			len += (size_t)n;
		close(fd);
	}
	res = malloc(DATASZ * 4);
	if (!res)
		return (NULL);
	l = 0;
	off = 0;
	while (off < len)
	{
		ch = len - off;
		if (ch > 16)
			ch = 16;
		start = l;
		l += (size_t)sprintf(res + l, "%07x", (unsigned int)off);
		for (i = 0; i < ch; i += 2)
		{
			b0 = data[off + i];
			b1 = 0;
			if (i + 1 < ch)
				b1 = data[off + i + 1];
			l += (size_t)sprintf(res + l, " %02x%02x", b1, b0);
		}
		while (l - start < 47)
			res[l++] = ' ';
		res[l++] = '\n';
		off += 16;
	}
	if (len > 0)
		l += (size_t)sprintf(res + l, "%07x\n", (unsigned int)len);
	res[l] = '\0';
	return (res);
}

/* ----------------------------------- ex00 -------------------------------- */

/* ft_display_file: == cat con 1 fichero; mensajes de error por stderr. */
static void	test_ex00(const char *repo)
{
	const char	*ex = "ex00";
	const char	*srcs = "ex00/ft_display_file.c";
	char		src0[PATHSZ];
	char		bin[PATHSZ];
	char		*out;
	char		*err;
	char		*pargv[4];
	int			sig;

	first_src(src0, sizeof(src0), repo, srcs);
	if (access(src0, R_OK) != 0)
	{
		t_skip(ex, "ft_display_file", "fuente no encontrada");
		return ;
	}
	xsnprintf(bin, sizeof(bin), "%s/ft_display_file", g_tmp);
	if (compile_prog(repo, srcs, bin))
	{
		t_ko(ex, "ft_display_file (no compila)");
		dump_log();
		return ;
	}
	/* display f1 == contenido de f1 */
	pargv[0] = bin;
	pargv[1] = g_f1;
	pargv[2] = NULL;
	run_prog(bin, pargv, NULL, &out, &err, &sig);
	check_out(ex, "display fichero == cat", out, F1_CONTENT, sig);
	free(out);
	free(err);
	/* sin args -> "File name missing." por stderr */
	pargv[1] = NULL;
	run_prog(bin, pargv, NULL, &out, &err, &sig);
	check_out(ex, "sin args -> File name missing.", err,
		"File name missing.", sig);
	free(out);
	free(err);
	/* demasiados args -> "Too many arguments." */
	pargv[1] = (char *)"a";
	pargv[2] = (char *)"b";
	pargv[3] = NULL;
	run_prog(bin, pargv, NULL, &out, &err, &sig);
	check_out(ex, "demasiados args -> Too many arguments.", err,
		"Too many arguments.", sig);
	free(out);
	free(err);
	/* fichero ilegible -> "Cannot read file." */
	pargv[1] = g_nope;
	pargv[2] = NULL;
	run_prog(bin, pargv, NULL, &out, &err, &sig);
	check_out(ex, "fichero ilegible -> Cannot read file.", err,
		"Cannot read file.", sig);
	free(out);
	free(err);
}

/* ----------------------------------- ex01 -------------------------------- */

/* ft_cat: 1 fichero, 2 ficheros, stdin, y caso de error (exit!=0 + stderr). */
static void	test_ex01(const char *repo)
{
	const char	*ex = "ex01";
	const char	*srcs = "ex01/ft_cat.c";
	const char	*files[2];
	char		src0[PATHSZ];
	char		bin[PATHSZ];
	char		*out;
	char		*err;
	char		*exp;
	char		*pargv[4];
	int			sig;
	int			code;

	first_src(src0, sizeof(src0), repo, srcs);
	if (access(src0, R_OK) != 0)
	{
		t_skip(ex, "ft_cat", "fuente no encontrada");
		return ;
	}
	xsnprintf(bin, sizeof(bin), "%s/ft_cat", g_tmp);
	if (compile_prog(repo, srcs, bin))
	{
		t_ko(ex, "ft_cat (no compila)");
		dump_log();
		return ;
	}
	/* 1 fichero == cat */
	pargv[0] = bin;
	pargv[1] = g_f1;
	pargv[2] = NULL;
	run_prog(bin, pargv, NULL, &out, &err, &sig);
	check_out(ex, "cat 1 fichero == cat", out, F1_CONTENT, sig);
	free(out);
	free(err);
	/* 2 ficheros == cat f1 f2 */
	files[0] = g_f1;
	files[1] = g_f2;
	exp = gen_cat(files, 2);
	pargv[1] = g_f1;
	pargv[2] = g_f2;
	pargv[3] = NULL;
	run_prog(bin, pargv, NULL, &out, &err, &sig);
	check_out(ex, "cat 2 ficheros == cat", out, exp, sig);
	free(exp);
	free(out);
	free(err);
	/* sin args: lee de stdin (alimentado con dup2 desde un fichero) */
	pargv[1] = NULL;
	run_prog(bin, pargv, g_stdin, &out, &err, &sig);
	check_out(ex, "cat desde stdin", out, "abc", sig);
	free(out);
	free(err);
	/* fichero inexistente: exit != 0 y mensaje por stderr */
	pargv[1] = g_nope;
	pargv[2] = NULL;
	code = run_prog(bin, pargv, NULL, &out, &err, &sig);
	if (!sig && code != 0 && err && *err)
		t_ok(ex, "cat error (exit != 0 + mensaje en stderr)");
	else
	{
		t_ko(ex, "cat error (exit != 0 + mensaje en stderr)");
		printf("       %scodigo=%d, stderr=\"", C_D, code);
		print_escaped(err);
		printf("\"%s\n", C_0);
	}
	free(out);
	free(err);
}

/* ----------------------------------- ex02 -------------------------------- */

/* ft_tail: -c N en sus variantes, N > tamaño y 2 ficheros con cabeceras. */
static void	test_ex02(const char *repo)
{
	const char	*ex = "ex02";
	const char	*srcs = "ex02/ft_tail.c ex02/ft_tail_utils.c";
	const char	*files[2];
	char		src0[PATHSZ];
	char		bin[PATHSZ];
	char		*out;
	char		*err;
	char		*exp;
	char		*pargv[6];
	int			sig;

	first_src(src0, sizeof(src0), repo, srcs);
	if (access(src0, R_OK) != 0)
	{
		t_skip(ex, "ft_tail", "fuente no encontrada");
		return ;
	}
	xsnprintf(bin, sizeof(bin), "%s/ft_tail", g_tmp);
	if (compile_prog(repo, srcs, bin))
	{
		t_ko(ex, "ft_tail (no compila)");
		dump_log();
		return ;
	}
	files[0] = g_f1;
	/* -c 5 (separado) */
	exp = gen_tail(files, 1, 5);
	pargv[0] = bin;
	pargv[1] = (char *)"-c";
	pargv[2] = (char *)"5";
	pargv[3] = g_f1;
	pargv[4] = NULL;
	run_prog(bin, pargv, NULL, &out, &err, &sig);
	check_out(ex, "tail -c 5", out, exp, sig);
	free(exp);
	free(out);
	free(err);
	/* -c5 (pegado) */
	exp = gen_tail(files, 1, 5);
	pargv[1] = (char *)"-c5";
	pargv[2] = g_f1;
	pargv[3] = NULL;
	run_prog(bin, pargv, NULL, &out, &err, &sig);
	check_out(ex, "tail -c5 (pegado)", out, exp, sig);
	free(exp);
	free(out);
	free(err);
	/* -c 999 (mayor que el tamaño: fichero completo) */
	exp = gen_tail(files, 1, 999);
	pargv[1] = (char *)"-c";
	pargv[2] = (char *)"999";
	pargv[3] = g_f1;
	pargv[4] = NULL;
	run_prog(bin, pargv, NULL, &out, &err, &sig);
	check_out(ex, "tail -c 999 (> tamaño)", out, exp, sig);
	free(exp);
	free(out);
	free(err);
	/* -c 3 con 2 ficheros: cabeceras "==> f <==" */
	files[1] = g_f2;
	exp = gen_tail(files, 2, 3);
	pargv[1] = (char *)"-c";
	pargv[2] = (char *)"3";
	pargv[3] = g_f1;
	pargv[4] = g_f2;
	pargv[5] = NULL;
	run_prog(bin, pargv, NULL, &out, &err, &sig);
	check_out(ex, "tail -c 3 (2 ficheros)", out, exp, sig);
	free(exp);
	free(out);
	free(err);
}

/* ----------------------------------- ex03 -------------------------------- */

/* ft_hexdump: 1 fichero, 2 ficheros (offset continuo) y fichero vacio. */
static void	test_ex03(const char *repo)
{
	const char	*ex = "ex03";
	const char	*srcs = "ex03/ft_hexdump.c ex03/ft_hexdump_utils.c";
	const char	*files[2];
	char		src0[PATHSZ];
	char		bin[PATHSZ];
	char		*out;
	char		*err;
	char		*exp;
	char		*pargv[4];
	int			sig;

	first_src(src0, sizeof(src0), repo, srcs);
	if (access(src0, R_OK) != 0)
	{
		t_skip(ex, "ft_hexdump", "fuente no encontrada");
		return ;
	}
	xsnprintf(bin, sizeof(bin), "%s/ft_hexdump", g_tmp);
	if (compile_prog(repo, srcs, bin))
	{
		t_ko(ex, "ft_hexdump (no compila)");
		dump_log();
		return ;
	}
	/* 1 fichero */
	files[0] = g_f1;
	exp = gen_hexdump(files, 1);
	pargv[0] = bin;
	pargv[1] = g_f1;
	pargv[2] = NULL;
	run_prog(bin, pargv, NULL, &out, &err, &sig);
	check_out(ex, "hexdump 1 fichero", out, exp, sig);
	free(exp);
	free(out);
	free(err);
	/* 2 ficheros: el offset continua entre ficheros */
	files[1] = g_f2;
	exp = gen_hexdump(files, 2);
	pargv[1] = g_f1;
	pargv[2] = g_f2;
	pargv[3] = NULL;
	run_prog(bin, pargv, NULL, &out, &err, &sig);
	check_out(ex, "hexdump 2 ficheros (offset continuo)", out, exp, sig);
	free(exp);
	free(out);
	free(err);
	/* fichero vacio: sin salida */
	pargv[1] = g_empty;
	pargv[2] = NULL;
	run_prog(bin, pargv, NULL, &out, &err, &sig);
	check_out(ex, "hexdump fichero vacio", out, "", sig);
	free(out);
	free(err);
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

/* Crea los ficheros de prueba en el dir temporal (nunca en el repo). */
static void	make_fixtures(void)
{
	xsnprintf(g_f1, sizeof(g_f1), "%s/f1", g_tmp);
	write_file(g_f1, F1_CONTENT);
	xsnprintf(g_f2, sizeof(g_f2), "%s/f2", g_tmp);
	write_file(g_f2, F2_CONTENT);
	xsnprintf(g_empty, sizeof(g_empty), "%s/empty", g_tmp);
	write_file(g_empty, "");
	xsnprintf(g_stdin, sizeof(g_stdin), "%s/stdinf", g_tmp);
	write_file(g_stdin, "abc");
	xsnprintf(g_nope, sizeof(g_nope), "%s/nope", g_tmp);
}

int	main(int argc, char **argv)
{
	const char	*repo;

	repo = (argc > 1) ? argv[1] : detect_repo();
	init_colors();
	strcpy(g_tmp, "/tmp/c10test.XXXXXX");
	if (!mkdtemp(g_tmp))
		return (perror("mkdtemp"), 1);
	make_fixtures();
	printf("repo: %s\n\n", repo);
	test_ex00(repo);
	test_ex01(repo);
	test_ex02(repo);
	test_ex03(repo);
	printf("\n%s%d OK%s  %s%d KO%s  %s%d SKIP%s\n",
		C_OK, g_ok, C_0, C_KO, g_ko, C_0, C_SK, g_skip, C_0);
	cleanup();
	return (g_ko != 0);
}
