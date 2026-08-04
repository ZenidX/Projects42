/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para los ejercicios de C01                      */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*              (o desde test/:  make)                                        */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*              (o desde test/:  make test)                                   */
/*                                                                            */
/*   El archivo tiene DOS modos, seleccionados por la macro FT_EX:            */
/*                                                                            */
/*   1. Modo runner (se compila sin -DFT_EX). Para cada ejercicio:            */
/*        a) compila repo/exNN/fuente.c con -Wall -Wextra -Werror             */
/*        b) avisa si el fuente todavia define un main()                      */
/*        c) recompila ESTE MISMO archivo con -DFT_EX=NN, haciendo #include   */
/*           del fuente del alumno y neutralizando su main con -Dmain=...     */
/*        d) lanza el binario resultante en un proceso hijo, con timeout,     */
/*           para que un segfault en un ejercicio no tumbe los demas          */
/*                                                                            */
/*   2. Modo test (-DFT_EX=NN). Solo se compila el bloque de casos del        */
/*      ejercicio NN. Cada caso compara contra una implementacion de          */
/*      referencia (la libc cuando existe equivalente).                       */
/*                                                                            */
/*   Anadir un ejercicio = una entrada en g_ex + un bloque #if FT_EX == NN.   */
/*                                                                            */
/* ************************************************************************** */

#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/* ========================================================================== */
/*                                MODO TEST                                   */
/* ========================================================================== */

#ifdef FT_EX

/* El fuente del alumno se incluye tal cual: asi el compilador ve la          */
/* definicion real y un prototipo equivocado sale como error de compilacion,  */
/* no como comportamiento indefinido en tiempo de ejecucion. Su main() llega  */
/* renombrado por -Dmain=...; lo deshacemos para declarar el nuestro.         */

#include FT_SRC
#undef main

# define CANARY '#'

static int			g_ok;
static int			g_ko;
static char			*g_got;
static int			g_saved_fd = -1;
static int			g_cap_fd = -1;
static const char	*C_OK = "";
static const char	*C_KO = "";
static const char	*C_D = "";
static const char	*C_0 = "";

static void	init_colors(void)
{
	if (!isatty(STDOUT_FILENO))
		return ;
	C_OK = "\033[32m";
	C_KO = "\033[31m";
	C_D = "\033[90m";
	C_0 = "\033[0m";
}

/* Devuelve s entrecomillado y con los no imprimibles escapados, en uno de    */
/* cuatro buffers rotatorios (para poder usarlo varias veces en un printf).   */
static const char	*q(const char *s) __attribute__((unused));

static const char	*q(const char *s)
{
	static char	buf[4][160];
	static int	turn;
	char		*d;
	size_t		k;

	d = buf[turn];
	turn = (turn + 1) % 4;
	if (!s)
		return ("NULL");
	k = 0;
	d[k++] = '"';
	while (*s && k < 150)
	{
		if (*s == '\n')
			k += (size_t)sprintf(d + k, "\\n");
		else if (*s == '\t')
			k += (size_t)sprintf(d + k, "\\t");
		else if (*s == '"' || *s == '\\')
			k += (size_t)sprintf(d + k, "\\%c", *s);
		else if ((unsigned char)*s < 32 || (unsigned char)*s > 126)
			k += (size_t)sprintf(d + k, "\\x%02x", (unsigned char)*s);
		else
			d[k++] = *s;
		s++;
	}
	if (*s)
		k += (size_t)sprintf(d + k, "...");
	d[k++] = '"';
	d[k] = '\0';
	return (d);
}

static int	res(int pass, const char *fmt, ...)
	__attribute__((format(printf, 2, 3), unused));

static int	res(int pass, const char *fmt, ...)
{
	va_list	ap;

	if (pass)
	{
		g_ok++;
		printf("  %s[OK]%s   ", C_OK, C_0);
	}
	else
	{
		g_ko++;
		printf("  %s[KO]%s   ", C_KO, C_0);
	}
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
	return (pass);
}

static void	detail(const char *fmt, ...)
	__attribute__((format(printf, 1, 2), unused));

static void	detail(const char *fmt, ...)
{
	va_list	ap;

	printf("         %s", C_D);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("%s\n", C_0);
}

static void	note_line(const char *note) __attribute__((unused));

static void	note_line(const char *note)
{
	if (note)
		detail("%s", note);
}

/* Indice del primer byte distinto entre a y b (n = tamano a comparar).       */
static size_t	first_diff(const char *a, const char *b, size_t n)
	__attribute__((unused));

static size_t	first_diff(const char *a, const char *b, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n && a[i] == b[i])
		i++;
	return (i);
}

/* -------------------- captura de la salida estandar ---------------------- */
/* Los ejercicios que "muestran" cosas escriben con write(1, ...), asi que no */
/* vale con redirigir el FILE *stdout: hay que redirigir el descriptor 1 a un */
/* fichero temporal y leerlo despues.                                         */

static void	cap_start(void) __attribute__((unused));

static void	cap_start(void)
{
	char	tmpl[] = "/tmp/fttestcapXXXXXX";

	fflush(stdout);
	g_saved_fd = dup(STDOUT_FILENO);
	g_cap_fd = mkstemp(tmpl);
	if (g_saved_fd < 0 || g_cap_fd < 0)
	{
		fprintf(stderr, "no se pudo capturar stdout\n");
		exit(1);
	}
	unlink(tmpl);
	dup2(g_cap_fd, STDOUT_FILENO);
}

/* Cierra la captura y deja el texto en g_got (siempre terminado en '\0').    */
static void	cap_end(void) __attribute__((unused));

static void	cap_end(void)
{
	off_t	n;
	ssize_t	r;

	fflush(stdout);
	n = lseek(STDOUT_FILENO, 0, SEEK_CUR);
	dup2(g_saved_fd, STDOUT_FILENO);
	close(g_saved_fd);
	g_saved_fd = -1;
	if (n < 0)
		n = 0;
	g_got = malloc((size_t)n + 1);
	if (!g_got)
		exit(1);
	lseek(g_cap_fd, 0, SEEK_SET);
	r = read(g_cap_fd, g_got, (size_t)n);
	if (r < 0)
		r = 0;
	g_got[r] = '\0';
	close(g_cap_fd);
	g_cap_fd = -1;
}

/* Compara la ultima captura con exp, reporta y libera g_got.                 */
static void	cmp_out(const char *exp, const char *fmt, ...)
	__attribute__((format(printf, 2, 3), unused));

static void	cmp_out(const char *exp, const char *fmt, ...)
{
	va_list	ap;
	size_t	lg;
	size_t	le;
	int		pass;

	lg = g_got ? strlen(g_got) : 0;
	le = strlen(exp);
	pass = (g_got && lg == le && memcmp(g_got, exp, le) == 0);
	if (pass)
	{
		g_ok++;
		printf("  %s[OK]%s   ", C_OK, C_0);
	}
	else
	{
		g_ko++;
		printf("  %s[KO]%s   ", C_KO, C_0);
	}
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
	if (!pass)
	{
		detail("esperado %s", q(exp));
		detail("obtenido %s", q(g_got));
		if (lg != le || lg > 60)
			detail("longitud esperada %zu, obtenida %zu; primer byte distinto "
				"en %zu", le, lg, first_diff(exp, g_got ? g_got : "",
					le < lg ? le : lg));
	}
	free(g_got);
	g_got = NULL;
}

/* -------------------------------- ex00 ----------------------------------- */

# if FT_EX == 0

static void	t_ft(int start)
{
	int	n;

	n = start;
	ft_ft(&n);
	if (!res(n == 42, "ft_ft(&n) con n = %d", start))
		detail("esperado 42, obtenido %d", n);
}

static void	run_cases(void)
{
	t_ft(0);
	t_ft(-1);
	t_ft(42);
	t_ft(INT_MAX);
}

# endif

/* -------------------------------- ex01 ----------------------------------- */

# if FT_EX == 1

static void	run_cases(void)
{
	int	n;
	int	*p1;
	int	**p2;
	int	***p3;
	int	****p4;
	int	*****p5;
	int	******p6;
	int	*******p7;
	int	********p8;
	int	*********p9;

	n = 0;
	p1 = &n;
	p2 = &p1;
	p3 = &p2;
	p4 = &p3;
	p5 = &p4;
	p6 = &p5;
	p7 = &p6;
	p8 = &p7;
	p9 = &p8;
	ft_ultimate_ft(p9);
	if (!res(n == 42, "ft_ultimate_ft(p9) con nueve niveles de puntero"))
		detail("esperado 42, obtenido %d", n);
}

# endif

/* -------------------------------- ex02 ----------------------------------- */

# if FT_EX == 2

static void	t_swap(int a, int b)
{
	int	x;
	int	y;

	x = a;
	y = b;
	ft_swap(&x, &y);
	if (!res(x == b && y == a, "ft_swap(&%d, &%d)", a, b))
		detail("esperado a=%d b=%d, obtenido a=%d b=%d", b, a, x, y);
}

static void	run_cases(void)
{
	t_swap(1, 2);
	t_swap(-5, 5);
	t_swap(0, 0);
	t_swap(42, 42);
	t_swap(INT_MIN, INT_MAX);
}

# endif

/* -------------------------------- ex03 ----------------------------------- */

# if FT_EX == 3

static void	t_div_mod(int a, int b)
{
	int	d;
	int	m;

	d = 12345;
	m = 12345;
	ft_div_mod(a, b, &d, &m);
	if (!res(d == a / b && m == a % b, "ft_div_mod(%d, %d, &div, &mod)", a, b))
		detail("esperado div=%d mod=%d, obtenido div=%d mod=%d",
			a / b, a % b, d, m);
}

static void	run_cases(void)
{
	t_div_mod(10, 3);
	t_div_mod(-10, 3);
	t_div_mod(10, -3);
	t_div_mod(-10, -3);
	t_div_mod(0, 5);
	t_div_mod(7, 7);
	t_div_mod(1, 2);
	t_div_mod(INT_MAX, 7);
	t_div_mod(INT_MIN, 7);
}

# endif

/* -------------------------------- ex04 ----------------------------------- */

# if FT_EX == 4

static void	t_ultimate_div_mod(int a, int b)
{
	int	x;
	int	y;

	x = a;
	y = b;
	ft_ultimate_div_mod(&x, &y);
	if (!res(x == a / b && y == a % b, "ft_ultimate_div_mod(&%d, &%d)", a, b))
		detail("esperado a=%d b=%d, obtenido a=%d b=%d", a / b, a % b, x, y);
}

static void	run_cases(void)
{
	t_ultimate_div_mod(10, 3);
	t_ultimate_div_mod(-10, 3);
	t_ultimate_div_mod(10, -3);
	t_ultimate_div_mod(-10, -3);
	t_ultimate_div_mod(0, 5);
	t_ultimate_div_mod(7, 7);
	t_ultimate_div_mod(1, 2);
	t_ultimate_div_mod(INT_MAX, 7);
}

# endif

/* -------------------------------- ex05 ----------------------------------- */

# if FT_EX == 5

static void	t_putstr(char *s)
{
	cap_start();
	ft_putstr(s);
	cap_end();
	cmp_out(s, "ft_putstr(%s)", q(s));
}

static void	run_cases(void)
{
	t_putstr("");
	t_putstr("a");
	t_putstr("Hola mundo");
	t_putstr("con\nsalto");
	t_putstr("  espacios  ");
	t_putstr("tabulador\ty fin");
}

# endif

/* -------------------------------- ex06 ----------------------------------- */

# if FT_EX == 6

static void	t_strlen(char *s)
{
	int	mine;
	int	real;

	mine = ft_strlen(s);
	real = (int)strlen(s);
	if (!res(mine == real, "ft_strlen(%s)", q(s)))
		detail("esperado %d, obtenido %d", real, mine);
}

static void	run_cases(void)
{
	t_strlen("");
	t_strlen("a");
	t_strlen("Hola");
	t_strlen("Hola mundo mundial");
	t_strlen("con\nsalto\ty tab");
	t_strlen("0123456789012345678901234567890123456789");
}

# endif

/* -------------------------------- ex07 ----------------------------------- */

# if FT_EX == 7

/* Formatea un array de enteros en uno de cuatro buffers rotatorios. */
static const char	*qa(int *tab, int size)
{
	static char	buf[4][256];
	static int	turn;
	char		*d;
	int			k;
	int			i;

	d = buf[turn];
	turn = (turn + 1) % 4;
	k = sprintf(d, "{");
	i = 0;
	while (i < size && k < 230)
	{
		k += sprintf(d + k, "%s%d", i ? ", " : "", tab[i]);
		i++;
	}
	if (i < size)
		k += sprintf(d + k, ", ...");
	sprintf(d + k, "}");
	return (d);
}

static void	t_rev(int *src, int size)
{
	int	mine[32];
	int	exp[32];
	int	i;

	memcpy(mine, src, (size_t)size * sizeof(int));
	i = 0;
	while (i < size)
	{
		exp[i] = src[size - 1 - i];
		i++;
	}
	ft_rev_int_tab(mine, size);
	if (!res(memcmp(mine, exp, (size_t)size * sizeof(int)) == 0,
			"ft_rev_int_tab(%s, %d)", qa(src, size), size))
		detail("esperado %s, obtenido %s", qa(exp, size), qa(mine, size));
}

static void	run_cases(void)
{
	int	a[] = {1, 2, 3, 4, 5};
	int	b[] = {1, 2, 3, 4};
	int	c[] = {7};
	int	d[] = {0, 0, 0};
	int	e[] = {INT_MIN, -1, 0, 1, INT_MAX};

	t_rev(a, 5);
	t_rev(b, 4);
	t_rev(c, 1);
	t_rev(c, 0);
	t_rev(d, 3);
	t_rev(e, 5);
}

# endif

/* -------------------------------- ex08 ----------------------------------- */

# if FT_EX == 8

static const char	*qa(int *tab, int size)
{
	static char	buf[4][256];
	static int	turn;
	char		*d;
	int			k;
	int			i;

	d = buf[turn];
	turn = (turn + 1) % 4;
	k = sprintf(d, "{");
	i = 0;
	while (i < size && k < 230)
	{
		k += sprintf(d + k, "%s%d", i ? ", " : "", tab[i]);
		i++;
	}
	if (i < size)
		k += sprintf(d + k, ", ...");
	sprintf(d + k, "}");
	return (d);
}

static int	cmp_int(const void *a, const void *b)
{
	int	x;
	int	y;

	x = *(const int *)a;
	y = *(const int *)b;
	return ((x > y) - (x < y));
}

static void	t_sort(int *src, int size)
{
	int	mine[32];
	int	exp[32];

	memcpy(mine, src, (size_t)size * sizeof(int));
	memcpy(exp, src, (size_t)size * sizeof(int));
	qsort(exp, (size_t)size, sizeof(int), cmp_int);
	ft_sort_int_tab(mine, size);
	if (!res(memcmp(mine, exp, (size_t)size * sizeof(int)) == 0,
			"ft_sort_int_tab(%s, %d)", qa(src, size), size))
		detail("esperado %s, obtenido %s", qa(exp, size), qa(mine, size));
}

static void	run_cases(void)
{
	int	a[] = {3, 1, 2};
	int	b[] = {5, 4, 3, 2, 1};
	int	c[] = {1, 2, 3, 4, 5};
	int	d[] = {7};
	int	e[] = {2, 2, 1, 1, 3};
	int	f[] = {INT_MAX, 0, INT_MIN, -1, 1};

	t_sort(a, 3);
	t_sort(b, 5);
	t_sort(c, 5);
	t_sort(d, 1);
	t_sort(d, 0);
	t_sort(e, 5);
	t_sort(f, 5);
}

# endif

/* ------------------------------- main hijo -------------------------------- */

int	main(void)
{
	/* sin buffer: si un caso revienta, lo ya impreso no se pierde */
	setvbuf(stdout, NULL, _IONBF, 0);
	init_colors();
	run_cases();
	if (g_ko > 250)
		return (250);
	return (g_ko);
}

#else

/* ========================================================================== */
/*                               MODO RUNNER                                  */
/* ========================================================================== */

# include <dirent.h>

# define PATHSZ 4096
# define TIMEOUT 20

typedef struct s_ex
{
	int			id;
	const char	*dir;
	const char	*src;
}	t_ex;

static const t_ex	g_ex[] = {
{0, "ex00", "ft_ft.c"},
{1, "ex01", "ft_ultimate_ft.c"},
{2, "ex02", "ft_swap.c"},
{3, "ex03", "ft_div_mod.c"},
{4, "ex04", "ft_ultimate_div_mod.c"},
{5, "ex05", "ft_putstr.c"},
{6, "ex06", "ft_strlen.c"},
{7, "ex07", "ft_rev_int_tab.c"},
{8, "ex08", "ft_sort_int_tab.c"},
};
# define N_EX (sizeof(g_ex) / sizeof(g_ex[0]))

static char			g_tmp[64];
static const char	*C_OK = "";
static const char	*C_KO = "";
static const char	*C_SK = "";
static const char	*C_B = "";
static const char	*C_D = "";
static const char	*C_0 = "";

static void	init_colors(void)
{
	if (!isatty(STDOUT_FILENO))
		return ;
	C_OK = "\033[32m";
	C_KO = "\033[31m";
	C_SK = "\033[33m";
	C_B = "\033[1m";
	C_D = "\033[90m";
	C_0 = "\033[0m";
}

/* snprintf que aborta si la ruta no cabe, en vez de truncar en silencio */
static void	xsnprintf(char *dst, size_t n, const char *fmt, ...)
	__attribute__((format(printf, 3, 4)));

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

static char	*slurp(const char *path)
{
	char	*txt;
	int		fd;

	fd = open(path, O_RDONLY);
	if (fd < 0)
		return (NULL);
	txt = read_all(fd);
	close(fd);
	return (txt);
}

static void	dump_log(const char *path)
{
	char	*log;

	log = slurp(path);
	if (log && *log)
		printf("%s%s%s", C_D, log, C_0);
	free(log);
}

/* Ruta absoluta de p (hace falta para el #include del fuente del alumno,     */
/* que se resuelve respecto al directorio de test.c, no al cwd).              */
static int	abs_path(const char *p, char *out, size_t n)
{
	char	cwd[PATHSZ];

	if (p[0] == '/')
	{
		xsnprintf(out, n, "%s", p);
		return (1);
	}
	if (!getcwd(cwd, sizeof(cwd)))
		return (0);
	xsnprintf(out, n, "%s/%s", cwd, p);
	return (1);
}

/* Localiza este mismo test.c, que hace falta para recompilarlo por           */
/* ejercicio. Primero junto al binario, luego rutas habituales.               */
static void	find_self(const char *argv0, char *out, size_t n)
{
	char	dir[PATHSZ];
	char	*slash;

	xsnprintf(dir, sizeof(dir), "%s", argv0);
	slash = strrchr(dir, '/');
	if (slash)
	{
		*slash = '\0';
		xsnprintf(out, n, "%s/test.c", dir);
		if (access(out, R_OK) == 0)
			return ;
	}
	if (access("test/test.c", R_OK) == 0)
		return (xsnprintf(out, n, "test/test.c"));
	if (access("test.c", R_OK) == 0)
		return (xsnprintf(out, n, "test.c"));
	xsnprintf(out, n, "%s", __FILE__);
}

/* Cuando falta el fuente que pide el subject, mira que .c hay en el          */
/* directorio: casi siempre el problema es el nombre del archivo.             */
static void	hint_other_sources(const char *repo, const t_ex *ex)
{
	char			path[PATHSZ];
	DIR				*d;
	struct dirent	*e;
	size_t			len;

	xsnprintf(path, sizeof(path), "%s/%s", repo, ex->dir);
	d = opendir(path);
	if (!d)
		return ;
	e = readdir(d);
	while (e)
	{
		len = strlen(e->d_name);
		if (len > 2 && strcmp(e->d_name + len - 2, ".c") == 0
			&& strcmp(e->d_name, ex->src) != 0)
			printf("  %s[AVISO]%s hay un %s: el subject pide exactamente %s\n",
				C_SK, C_0, e->d_name, ex->src);
		e = readdir(d);
	}
	closedir(d);
}

/* Escribe un stub con main() y lo compila. Sirve para detectar despues si    */
/* el fuente del alumno tambien define main (el linker se quejaria).          */
static void	make_stub(void)
{
	char	path[PATHSZ];
	char	cmd[PATHSZ * 2];
	FILE	*f;

	xsnprintf(path, sizeof(path), "%s/stub.c", g_tmp);
	f = fopen(path, "w");
	if (!f)
		return ;
	fprintf(f, "int main(void){return (0);}\n");
	fclose(f);
	xsnprintf(cmd, sizeof(cmd), "cc -c -o '%s/stub.o' '%s' 2>/dev/null",
		g_tmp, path);
	if (system(cmd) != 0)
		fprintf(stderr, "aviso: no se pudo preparar la deteccion de main\n");
}

static void	warn_if_main(const char *obj)
{
	char	cmd[PATHSZ * 3];
	char	log[PATHSZ];
	char	*txt;

	xsnprintf(log, sizeof(log), "%s/mainchk.log", g_tmp);
	xsnprintf(cmd, sizeof(cmd),
		"cc -o '%s/mainchk.bin' '%s' '%s/stub.o' > '%s' 2>&1",
		g_tmp, obj, g_tmp, log);
	if (system(cmd) == 0)
		return ;
	txt = slurp(log);
	if (txt && (strstr(txt, "multiple definition")
			|| strstr(txt, "duplicate symbol")))
		printf("  %s[AVISO]%s el fuente todavia define main(): quitalo antes "
			"de entregar o la moulinette dara \"duplicate symbol _main\"\n",
			C_SK, C_0);
	free(txt);
}

/* Lanza bin en un hijo con timeout. Devuelve el numero de tests KO que       */
/* reporto, o -1 si murio por una senal.                                      */
static int	run_child(const char *bin)
{
	pid_t	pid;
	int		status;

	pid = fork();
	if (pid == -1)
		return (perror("fork"), -1);
	if (pid == 0)
	{
		alarm(TIMEOUT);
		execl(bin, bin, (char *)NULL);
		_exit(127);
	}
	if (waitpid(pid, &status, 0) == -1)
		return (-1);
	if (WIFSIGNALED(status))
	{
		printf("  %s[CRASH]%s el test muere con senal %d (%s)\n",
			C_KO, C_0, WTERMSIG(status), strsignal(WTERMSIG(status)));
		return (-1);
	}
	if (WEXITSTATUS(status) == 127)
		return (printf("  %s[KO]%s no se pudo ejecutar el test\n",
				C_KO, C_0), -1);
	return (WEXITSTATUS(status));
}

/* Devuelve el numero de tests KO, -1 si el ejercicio no se pudo probar,      */
/* -2 si el fuente no existe (SKIP).                                          */
static int	do_ex(const char *repo, const char *self, const t_ex *ex)
{
	char	src[PATHSZ];
	char	abs[PATHSZ];
	char	obj[PATHSZ];
	char	bin[PATHSZ];
	char	log[PATHSZ];
	char	cmd[PATHSZ * 4];

	printf("%s── %s/%s%s\n", C_B, ex->dir, ex->src, C_0);
	xsnprintf(src, sizeof(src), "%s/%s/%s", repo, ex->dir, ex->src);
	if (access(src, R_OK) != 0 || !abs_path(src, abs, sizeof(abs)))
	{
		printf("  %s[SKIP]%s fuente no encontrado (%s)\n", C_SK, C_0, src);
		hint_other_sources(repo, ex);
		return (-2);
	}
	xsnprintf(log, sizeof(log), "%s/%s.log", g_tmp, ex->dir);
	xsnprintf(obj, sizeof(obj), "%s/%s.o", g_tmp, ex->dir);
	xsnprintf(bin, sizeof(bin), "%s/%s.bin", g_tmp, ex->dir);
	xsnprintf(cmd, sizeof(cmd),
		"cc -Wall -Wextra -Werror -c -o '%s' '%s' > '%s' 2>&1", obj, abs, log);
	if (system(cmd) != 0)
	{
		printf("  %s[COMPILA KO]%s con -Wall -Wextra -Werror\n", C_KO, C_0);
		dump_log(log);
		return (-1);
	}
	warn_if_main(obj);
	/* -Wno-return-type: al renombrar main() deja de aplicarsele el trato
	   especial del estandar y "cae" sin return; ya se comprobo intacto arriba */
	xsnprintf(cmd, sizeof(cmd),
		"cc -Wall -Wextra -Werror -Wno-return-type -DFT_EX=%d -DFT_SRC='\"%s\"' "
		"-Dmain=ft_test_unused_main -o '%s' '%s' > '%s' 2>&1",
		ex->id, abs, bin, self, log);
	if (system(cmd) != 0)
	{
		printf("  %s[TEST KO]%s no se pudo montar el test (prototipo distinto "
			"del que pide el subject?)\n", C_KO, C_0);
		dump_log(log);
		return (-1);
	}
	return (run_child(bin));
}

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
	char		self[PATHSZ];
	int			ko_tests;
	int			ko_ex;
	int			skipped;
	int			r;
	size_t		i;

	repo = NULL;
	i = 1;
	while ((int)i < argc)
		repo = argv[i++];
	if (!repo)
		repo = (access("repo", X_OK) == 0) ? "repo" : "../repo";
	/* sin buffer: los hijos escriben en este mismo stdout, y si el padre
	   acumulase su salida se imprimiria toda al final, desordenada */
	setvbuf(stdout, NULL, _IONBF, 0);
	init_colors();
	find_self(argv[0], self, sizeof(self));
	strcpy(g_tmp, "/tmp/c01test.XXXXXX");
	if (!mkdtemp(g_tmp))
		return (perror("mkdtemp"), 1);
	make_stub();
	printf("repo: %s\ntest: %s\n\n", repo, self);
	ko_tests = 0;
	ko_ex = 0;
	skipped = 0;
	i = 0;
	while (i < N_EX)
	{
		r = do_ex(repo, self, &g_ex[i++]);
		if (r == -2)
			skipped++;
		else if (r != 0)
			ko_ex++;
		if (r > 0)
			ko_tests += r;
		printf("\n");
	}
	printf("%s%zu ejercicios%s  %s%d con fallos%s  %s%d sin fuente%s",
		C_B, N_EX, C_0, C_KO, ko_ex, C_0, C_SK, skipped, C_0);
	if (ko_tests)
		printf("  %s(%d tests KO)%s", C_KO, ko_tests, C_0);
	printf("\n");
	cleanup();
	return (ko_ex != 0);
}

#endif
