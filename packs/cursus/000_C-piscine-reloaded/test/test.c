/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para C Piscine Reloaded                         */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*              (o desde test/:  make)                                        */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*              (o desde test/:  make test)                                   */
/*                                                                            */
/*   Reloaded mezcla dos clases de ejercicio, asi que el runner tiene dos      */
/*   clases de comprobacion:                                                  */
/*                                                                            */
/*   a) SHELL (ex00-ex05). No hay nada que compilar: lo que se entrega es un   */
/*      tar, una linea de comando o un archivo con un nombre imposible. Cada   */
/*      uno se comprueba a su manera — se extrae el tar y se miran permisos,   */
/*      tamanos y enlaces; se monta un arbol de prueba en /tmp y se ejecuta    */
/*      la linea de comando dentro para ver que borra lo que debe y respeta    */
/*      lo que no. Nunca se ejecuta nada dentro del repo del alumno.           */
/*                                                                            */
/*   b) C (ex06 en adelante). Como en los packs de la piscine: el fuente se    */
/*      compila con -Wall -Wextra -Werror, este mismo archivo se recompila con */
/*      -DFT_EX=NN haciendo #include de el, y los casos corren en un hijo con  */
/*      timeout para que un segfault no tumbe la tanda.                       */
/*                                                                            */
/*   Anadir un ejercicio de C = una entrada {NN, "exNN", "fuente.c", EX_C} en  */
/*   g_ex + un bloque #if FT_EX == NN con sus casos. Uno de shell = la entrada */
/*   con EX_SHELL + su rama en do_shell_ex().                                 */
/*                                                                            */
/* ************************************************************************** */

#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* ========================================================================== */
/*                                MODO TEST                                   */
/* ========================================================================== */

#ifdef FT_EX

/* El fuente del alumno se incluye tal cual: asi el compilador ve la          */
/* definicion real y un prototipo equivocado sale como error de compilacion,  */
/* no como comportamiento indefinido en tiempo de ejecucion. Su main() llega  */
/* renombrado por -Dmain=...; lo deshacemos para declarar el nuestro.         */

/* Si el subject pide dos archivos (p.ej. ft_convert_base.c y su "2"), el     */
/* segundo se incluye primero: suele traer los helpers que usa el principal,  */
/* y asi ya estan definidos en la primera llamada.                            */
#ifdef FT_SRC2
# include FT_SRC2
#endif
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

/* ---------------------------- caso en curso ------------------------------ */
/* Cada caso se anuncia con CASE(...) ANTES de llamar al codigo del alumno,   */
/* asi que si revienta el padre puede decir en cual fue: el hijo deja el caso */
/* en curso en FT_CASEFILE. Al relanzarlo con FT_SKIP=n se salta los n        */
/* primeros casos y sigue por el siguiente, que es como se llega al final de  */
/* la tanda aunque uno segfaultee.                                            */

static char			g_case[192];
static int			g_case_n;
static int			g_case_skip;
static const char	*g_case_file;

static void	case_init(void)
{
	const char	*s;

	s = getenv("FT_SKIP");
	if (s)
		g_case_skip = atoi(s);
	g_case_file = getenv("FT_CASEFILE");
}

static void	case_set(const char *fmt, va_list ap)
{
	int	fd;

	vsnprintf(g_case, sizeof(g_case), fmt, ap);
	if (!g_case_file)
		return ;
	fd = open(g_case_file, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd >= 0)
	{
		dprintf(fd, "%d\t%s\n", g_case_n - 1, g_case);
		close(fd);
	}
}

/* Afina la etiqueta del caso en curso: mismo caso, mas detalle. Para los     */
/* helpers que comprueban varias cosas de una sola llamada.                   */
static void	case_label(const char *fmt, ...)
	__attribute__((format(printf, 1, 2), unused));

static void	case_label(const char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	case_set(fmt, ap);
	va_end(ap);
}

/* Devuelve 1 si este caso ya se probo en un intento anterior (hay que        */
/* saltarselo), 0 si toca ejecutarlo.                                         */
static int	case_begin(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));

static int	case_begin(const char *fmt, ...)
{
	va_list	ap;

	if (g_case_n++ < g_case_skip)
		return (1);
	va_start(ap, fmt);
	case_set(fmt, ap);
	va_end(ap);
	return (0);
}

# define CASE(...) do { if (case_begin(__VA_ARGS__)) return ; } while (0)

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

/* Reporta el caso anunciado con CASE(). */
static int	res(int pass) __attribute__((unused));

static int	res(int pass)
{
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
	printf("%s\n", g_case);
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
static void	cmp_out(const char *exp) __attribute__((unused));

static void	cmp_out(const char *exp)
{
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
	printf("%s\n", g_case);
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

/* -------------------------------- ex06 ----------------------------------- */

# if FT_EX == 6

static void	run_cases(void)
{
	CASE("ft_print_alphabet()");
	cap_start();
	ft_print_alphabet();
	cap_end();
	cmp_out("abcdefghijklmnopqrstuvwxyz");
}

# endif

/* -------------------------------- ex07 ----------------------------------- */

# if FT_EX == 7

static void	run_cases(void)
{
	CASE("ft_print_numbers()");
	cap_start();
	ft_print_numbers();
	cap_end();
	cmp_out("0123456789");
}

# endif

/* -------------------------------- ex08 ----------------------------------- */

# if FT_EX == 8

static void	t_is_negative(int n)
{
	CASE("ft_is_negative(%d)", n);
	cap_start();
	ft_is_negative(n);
	cap_end();
	cmp_out(n < 0 ? "N" : "P");
}

static void	run_cases(void)
{
	t_is_negative(0);
	t_is_negative(1);
	t_is_negative(-1);
	t_is_negative(42);
	t_is_negative(-42);
	t_is_negative(INT_MAX);
	t_is_negative(INT_MIN);
}

# endif

/* -------------------------------- ex09 ----------------------------------- */

# if FT_EX == 9

static void	t_ft(int start)
{
	int	n;

	n = start;
	CASE("ft_ft(&n) con n = %d", start);
	ft_ft(&n);
	if (!res(n == 42))
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

/* -------------------------------- ex10 ----------------------------------- */

# if FT_EX == 10

static void	t_swap(int a, int b)
{
	int	x;
	int	y;

	x = a;
	y = b;
	CASE("ft_swap(&a, &b) con a = %d, b = %d", a, b);
	ft_swap(&x, &y);
	if (!res(x == b && y == a))
		detail("esperado a = %d y b = %d, obtenido a = %d y b = %d",
			b, a, x, y);
}

/* Con los dos punteros iguales, un swap por XOR mal hecho deja un 0. */
static void	t_swap_same(void)
{
	int	v;

	v = 7;
	CASE("ft_swap(&a, &a) deja el valor como estaba");
	ft_swap(&v, &v);
	if (!res(v == 7))
		detail("esperado 7, obtenido %d", v);
}

static void	run_cases(void)
{
	t_swap(1, 2);
	t_swap(0, 0);
	t_swap(-1, 1);
	t_swap(42, -42);
	t_swap(INT_MIN, INT_MAX);
	t_swap_same();
}

# endif

/* -------------------------------- ex11 ----------------------------------- */

# if FT_EX == 11

static void	t_div_mod(int a, int b)
{
	int	d;
	int	m;

	d = 0;
	m = 0;
	CASE("ft_div_mod(%d, %d, &div, &mod)", a, b);
	ft_div_mod(a, b, &d, &m);
	if (!res(d == a / b && m == a % b))
		detail("esperado div = %d y mod = %d, obtenido div = %d y mod = %d",
			a / b, a % b, d, m);
}

static void	run_cases(void)
{
	t_div_mod(10, 3);
	t_div_mod(-10, 3);
	t_div_mod(10, -3);
	t_div_mod(-10, -3);
	t_div_mod(0, 5);
	t_div_mod(42, 42);
	t_div_mod(1, 7);
	t_div_mod(INT_MAX, 2);
}

# endif

/* ----------------------------- ex12 y ex13 -------------------------------- */
/* El subject pide 0 "si hay un error": lo unico que lo es sin ambiguedad es  */
/* un nb negativo. El desbordamiento (13! ya no cabe en un int) no se prueba: */
/* el subject no dice que devolver y la moulinette tampoco lo mira.           */

# if FT_EX == 12

static void	t_fact(int n, int exp)
{
	int	r;

	CASE("ft_iterative_factorial(%d)", n);
	r = ft_iterative_factorial(n);
	if (!res(r == exp))
		detail("esperado %d, obtenido %d", exp, r);
}

static void	run_cases(void)
{
	t_fact(0, 1);
	t_fact(1, 1);
	t_fact(2, 2);
	t_fact(3, 6);
	t_fact(5, 120);
	t_fact(10, 3628800);
	t_fact(12, 479001600);
	t_fact(-1, 0);
	t_fact(-42, 0);
	t_fact(INT_MIN, 0);
}

# endif

# if FT_EX == 13

static void	t_fact(int n, int exp)
{
	int	r;

	CASE("ft_recursive_factorial(%d)", n);
	r = ft_recursive_factorial(n);
	if (!res(r == exp))
		detail("esperado %d, obtenido %d", exp, r);
}

static void	run_cases(void)
{
	t_fact(0, 1);
	t_fact(1, 1);
	t_fact(2, 2);
	t_fact(3, 6);
	t_fact(5, 120);
	t_fact(10, 3628800);
	t_fact(12, 479001600);
	t_fact(-1, 0);
	t_fact(-42, 0);
	t_fact(INT_MIN, 0);
}

# endif

/* -------------------------------- ex14 ----------------------------------- */

# if FT_EX == 14

static void	t_sqrt(int n, int exp)
{
	int	r;

	CASE("ft_sqrt(%d)", n);
	r = ft_sqrt(n);
	if (!res(r == exp))
		detail("esperado %d, obtenido %d", exp, r);
}

static void	run_cases(void)
{
	t_sqrt(0, 0);
	t_sqrt(1, 1);
	t_sqrt(4, 2);
	t_sqrt(9, 3);
	t_sqrt(144, 12);
	t_sqrt(1024, 32);
	t_sqrt(2, 0);
	t_sqrt(3, 0);
	t_sqrt(15, 0);
	t_sqrt(-4, 0);
	t_sqrt(-1, 0);
	/* el cuadrado perfecto mas grande que cabe en un int, y el propio tope:
	   aqui es donde un bucle i * i se desborda y no termina nunca */
	t_sqrt(2147395600, 46340);
	t_sqrt(INT_MAX, 0);
}

# endif

/* -------------------------------- ex15 ----------------------------------- */

# if FT_EX == 15

static void	t_putstr(char *s)
{
	CASE("ft_putstr(%s)", q(s));
	cap_start();
	ft_putstr(s);
	cap_end();
	cmp_out(s);
}

static void	run_cases(void)
{
	char	a[] = "";
	char	b[] = "hola";
	char	c[] = "con salto\n";
	char	d[] = "42 42 42";
	char	e[] = "\tcon tabulador y acentos raros: \xc3\xb1";

	t_putstr(b);
	t_putstr(a);
	t_putstr(c);
	t_putstr(d);
	t_putstr(e);
}

# endif

/* -------------------------------- ex16 ----------------------------------- */

# if FT_EX == 16

static void	t_strlen(char *s)
{
	int	r;

	CASE("ft_strlen(%s)", q(s));
	r = ft_strlen(s);
	if (!res(r == (int)strlen(s)))
		detail("esperado %d, obtenido %d", (int)strlen(s), r);
}

static void	run_cases(void)
{
	char	a[] = "";
	char	b[] = "a";
	char	c[] = "hola";
	char	d[] = "cadena bastante mas larga, con espacios y 42";
	char	e[] = "con\nsaltos\ty tabuladores";

	t_strlen(a);
	t_strlen(b);
	t_strlen(c);
	t_strlen(d);
	t_strlen(e);
}

# endif

/* -------------------------------- ex17 ----------------------------------- */
/* strcmp solo promete el signo, no el valor exacto, asi que se compara eso.  */

# if FT_EX == 17

static int	sgn(int n)
{
	if (n > 0)
		return (1);
	if (n < 0)
		return (-1);
	return (0);
}

static void	t_strcmp(char *a, char *b)
{
	int	got;
	int	exp;

	CASE("ft_strcmp(%s, %s)", q(a), q(b));
	got = ft_strcmp(a, b);
	exp = strcmp(a, b);
	if (!res(sgn(got) == sgn(exp)))
		detail("esperado signo %d (strcmp devuelve %d), obtenido %d",
			sgn(exp), exp, got);
}

static void	run_cases(void)
{
	char	vacia[] = "";
	char	a[] = "a";
	char	b[] = "b";
	char	hola[] = "hola";
	char	hola2[] = "hola";
	char	holaa[] = "holaa";
	char	alto[] = "\xff";

	t_strcmp(hola, hola2);
	t_strcmp(vacia, vacia);
	t_strcmp(a, b);
	t_strcmp(b, a);
	t_strcmp(hola, holaa);
	t_strcmp(holaa, hola);
	t_strcmp(vacia, a);
	t_strcmp(a, vacia);
	/* con char firmado, \xff sale negativo y el signo se invierte */
	t_strcmp(alto, a);
	t_strcmp(a, alto);
}

# endif

/* -------------------------------- ex20 ----------------------------------- */

# if FT_EX == 20

static void	t_strdup(char *s)
{
	char	*d;

	CASE("ft_strdup(%s)", q(s));
	d = ft_strdup(s);
	if (!d)
	{
		res(0);
		detail("devolvio NULL");
		return ;
	}
	if (!res(d != s && strcmp(d, s) == 0))
	{
		detail("esperado %s", q(s));
		detail("obtenido %s%s", q(d), d == s ? " (es el mismo puntero)" : "");
	}
	free(d);
}

/* La copia tiene que ser suya: tocarla no puede mover el original. */
static void	t_strdup_indep(void)
{
	char	src[] = "42";
	char	*d;

	CASE("la copia es independiente del original");
	d = ft_strdup(src);
	if (!d)
	{
		res(0);
		detail("devolvio NULL");
		return ;
	}
	d[0] = 'X';
	if (!res(src[0] == '4'))
		detail("al tocar la copia se movio el original: %s", q(src));
	free(d);
}

static void	run_cases(void)
{
	char	a[] = "";
	char	b[] = "hola";
	char	c[] = "una cadena con espacios y un 42 dentro";

	t_strdup(b);
	t_strdup(a);
	t_strdup(c);
	t_strdup_indep();
}

# endif

/* -------------------------------- ex21 ----------------------------------- */

# if FT_EX == 21

static void	t_range_null(int min, int max)
{
	int	*r;

	CASE("ft_range(%d, %d) con min >= max devuelve NULL", min, max);
	r = ft_range(min, max);
	if (!res(r == NULL))
		detail("devolvio un puntero en vez de NULL");
	free(r);
}

static void	t_range(int min, int max)
{
	int	*r;
	int	i;
	int	bad;

	CASE("ft_range(%d, %d)", min, max);
	r = ft_range(min, max);
	if (!r)
	{
		res(0);
		detail("devolvio NULL");
		return ;
	}
	i = 0;
	bad = -1;
	while (i < max - min)
	{
		if (r[i] != min + i && bad < 0)
			bad = i;
		i++;
	}
	if (!res(bad < 0))
		detail("en la posicion %d esperaba %d y hay %d", bad, min + bad,
			r[bad]);
	free(r);
}

static void	run_cases(void)
{
	t_range(0, 5);
	t_range(1, 2);
	t_range(-3, 3);
	t_range(-10, -5);
	t_range(40, 43);
	t_range_null(5, 5);
	t_range_null(5, 1);
	t_range_null(0, -3);
}

# endif

/* -------------------------------- ex25 ----------------------------------- */

# if FT_EX == 25

static int	g_seen[64];
static int	g_nseen;

static void	collect(int n)
{
	if (g_nseen < 64)
		g_seen[g_nseen] = n;
	g_nseen++;
}

static void	t_foreach(int *tab, int len)
{
	int	i;
	int	bad;

	g_nseen = 0;
	CASE("ft_foreach(tab, %d, f)", len);
	ft_foreach(tab, len, &collect);
	if (g_nseen != len)
	{
		res(0);
		detail("esperadas %d llamadas a f, hechas %d", len, g_nseen);
		return ;
	}
	i = 0;
	bad = -1;
	while (i < len)
	{
		if (g_seen[i] != tab[i] && bad < 0)
			bad = i;
		i++;
	}
	if (!res(bad < 0))
		detail("en la llamada %d esperaba %d y llego %d", bad, tab[bad],
			g_seen[bad]);
}

static void	run_cases(void)
{
	int	tab[] = {1, 2, 3, 42, -7};
	int	uno[] = {42};

	t_foreach(tab, 5);
	t_foreach(uno, 1);
	t_foreach(tab, 0);
	t_foreach(tab, 3);
}

# endif

/* -------------------------------- ex26 ----------------------------------- */

# if FT_EX == 26

static int	es_42(char *s)
{
	return (strcmp(s, "42") == 0);
}

static int	no_vacia(char *s)
{
	return (s[0] != '\0');
}

static void	t_count_if(char **tab, int (*f)(char *), const char *fname,
		int exp)
{
	int	r;

	CASE("ft_count_if(tab, %s)", fname);
	r = ft_count_if(tab, f);
	if (!res(r == exp))
		detail("esperado %d, obtenido %d", exp, r);
}

static void	run_cases(void)
{
	char	*tab[] = {"42", "x", "42", "", NULL};
	char	*vacio[] = {NULL};
	char	*ninguno[] = {"a", "b", NULL};

	t_count_if(tab, &es_42, "es_42", 2);
	t_count_if(tab, &no_vacia, "no_vacia", 3);
	t_count_if(vacio, &es_42, "es_42", 0);
	t_count_if(vacio, &no_vacia, "no_vacia", 0);
	t_count_if(ninguno, &es_42, "es_42", 0);
}

# endif

/* ------------------------------- main hijo -------------------------------- */

int	main(void)
{
	/* sin buffer: si un caso revienta, lo ya impreso no se pierde */
	setvbuf(stdout, NULL, _IONBF, 0);
	init_colors();
	case_init();
	run_cases();
	if (g_ko > 250)
		return (250);
	return (g_ko);
}

#else

/* ========================================================================== */
/*                               MODO RUNNER                                  */
/* ========================================================================== */

# include <ctype.h>
# include <dirent.h>
# include <signal.h>

# define PATHSZ 4096
# define TIMEOUT 20
/* topes de relanzamiento por ejercicio: si revientan (o se cuelgan) mas casos
   que esto, se corta la tanda en vez de seguir reintentando */
# define MAX_CRASH 12
# define MAX_HANG 1

/* EX_SHELL no compila nada: lo que se entrega no es codigo. */
enum
{
	EX_C = 0,
	EX_SHELL = 1,
	EX_PROG = 2,
	EX_HDR = 3,
	EX_MAKE = 4
};

typedef struct s_ex
{
	int			id;
	const char	*dir;
	const char	*src;
	const char	*src2;
	int			kind;
}	t_ex;

static const t_ex	g_ex[] = {
{0, "ex00", "exo.tar", NULL, EX_SHELL},
{1, "ex01", "z", NULL, EX_SHELL},
{2, "ex02", "clean", NULL, EX_SHELL},
{3, "ex03", "find_sh.sh", NULL, EX_SHELL},
{4, "ex04", "MAC.sh", NULL, EX_SHELL},
{5, "ex05", "\"\\?$*'MaRViN'*$?\\\"", NULL, EX_SHELL},
{6, "ex06", "ft_print_alphabet.c", NULL, EX_C},
{7, "ex07", "ft_print_numbers.c", NULL, EX_C},
{8, "ex08", "ft_is_negative.c", NULL, EX_C},
{9, "ex09", "ft_ft.c", NULL, EX_C},
{10, "ex10", "ft_swap.c", NULL, EX_C},
{11, "ex11", "ft_div_mod.c", NULL, EX_C},
{12, "ex12", "ft_iterative_factorial.c", NULL, EX_C},
{13, "ex13", "ft_recursive_factorial.c", NULL, EX_C},
{14, "ex14", "ft_sqrt.c", NULL, EX_C},
{15, "ex15", "ft_putstr.c", NULL, EX_C},
{16, "ex16", "ft_strlen.c", NULL, EX_C},
{17, "ex17", "ft_strcmp.c", NULL, EX_C},
{18, "ex18", "ft_print_params.c", NULL, EX_PROG},
{19, "ex19", "ft_sort_params.c", NULL, EX_PROG},
{20, "ex20", "ft_strdup.c", NULL, EX_C},
{21, "ex21", "ft_range.c", NULL, EX_C},
{22, "ex22", "ft_abs.h", NULL, EX_HDR},
{23, "ex23", "ft_point.h", NULL, EX_HDR},
{24, "ex24", "Makefile", NULL, EX_MAKE},
{25, "ex25", "ft_foreach.c", NULL, EX_C},
{26, "ex26", "ft_count_if.c", NULL, EX_C},
{27, "ex27", "Makefile", NULL, EX_MAKE},
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

/* Lanza bin en un hijo con timeout, saltandose los skip primeros casos.      */
/* Devuelve el numero de tests KO que reporto, -1 si no se pudo ejecutar, o   */
/* -2 si murio por una senal (que deja en *sig).                              */
static int	run_child(const char *bin, int skip, const char *casefile, int *sig)
{
	char	buf[16];
	pid_t	pid;
	int		status;

	pid = fork();
	if (pid == -1)
		return (perror("fork"), -1);
	if (pid == 0)
	{
		xsnprintf(buf, sizeof(buf), "%d", skip);
		setenv("FT_SKIP", buf, 1);
		setenv("FT_CASEFILE", casefile, 1);
		/* alarm() acota el tiempo, no la memoria: un bucle infinito con
		   malloc dentro se lleva la RAM de la maquina antes de que salte.
		   Con el techo, malloc devuelve NULL y el caso falla en el acto. */
		{
			struct rlimit	lim;

			lim.rlim_cur = 256UL * 1024 * 1024;
			lim.rlim_max = 256UL * 1024 * 1024;
			setrlimit(RLIMIT_AS, &lim);
			lim.rlim_cur = 0;
			lim.rlim_max = 0;
			setrlimit(RLIMIT_CORE, &lim);
		}
		alarm(TIMEOUT);
		execl(bin, bin, (char *)NULL);
		_exit(127);
	}
	if (waitpid(pid, &status, 0) == -1)
		return (-1);
	if (WIFSIGNALED(status))
		return (*sig = WTERMSIG(status), -2);
	if (WEXITSTATUS(status) == 127)
		return (printf("  %s[KO]%s no se pudo ejecutar el test\n",
				C_KO, C_0), -1);
	return (WEXITSTATUS(status));
}

/* Lee el caso que el hijo dejo anotado antes de reventar. Devuelve su indice */
/* y copia la descripcion en label, o -1 si no llego a anotar ninguno.        */
static int	read_case(const char *path, char *label, size_t n)
{
	char	*txt;
	char	*tab;
	int		idx;

	txt = slurp(path);
	if (!txt)
		return (-1);
	tab = strchr(txt, '\t');
	if (!tab)
		return (free(txt), -1);
	*tab++ = '\0';
	idx = atoi(txt);
	tab[strcspn(tab, "\n")] = '\0';
	snprintf(label, n, "%s", tab);
	return (free(txt), idx);
}

/* Un caso que revienta se lleva por delante el proceso entero, asi que el    */
/* resto de la tanda se quedaba sin probar. Aqui se relanza el binario justo  */
/* por detras del caso que murio, tantas veces como haga falta (con tope),    */
/* de modo que el recuento final sale completo y cada crash sale con nombre.  */
static int	run_bin(const char *bin, const char *casefile)
{
	char	label[192];
	int		ko;
	int		skip;
	int		crashes;
	int		hangs;
	int		idx;
	int		sig;
	int		r;

	ko = 0;
	skip = 0;
	crashes = 0;
	hangs = 0;
	while (1)
	{
		remove(casefile);
		sig = 0;
		r = run_child(bin, skip, casefile, &sig);
		if (r >= 0)
			return (ko + r);
		if (r == -1)
			return (ko > 0 ? ko : -1);
		idx = read_case(casefile, label, sizeof(label));
		if (idx < 0)
			return (printf("  %s[CRASH]%s el test muere con senal %d (%s) "
					"antes de llegar a ningun caso\n", C_KO, C_0, sig,
					strsignal(sig)), ko > 0 ? ko : -1);
		printf("  %s[CRASH]%s %s   %s%s%s\n", C_KO, C_0, label, C_KO,
			sig == SIGALRM ? "se cuelga (timeout)" : strsignal(sig), C_0);
		ko++;
		hangs += (sig == SIGALRM);
		/* cada cuelgue cuesta TIMEOUT segundos, asi que con esos se es mas
		   tacano que con los segfaults, que se reintentan al momento */
		if (idx < skip || ++crashes > MAX_CRASH || hangs >= MAX_HANG)
			return (printf("  %s[AVISO]%s los casos que quedan no se prueban\n",
					C_SK, C_0), ko);
		skip = idx + 1;
	}
}

/* Segundo archivo del ejercicio (los que el subject pide por parejas). Se    */
/* comprueba que existe y que compila suelto, igual que la moulinette, y se   */
/* devuelve su ruta absoluta en out. 0 = no se puede seguir.                  */
static int	second_source(const char *repo, const t_ex *ex, char *out,
		size_t n, const char *log)
{
	char	path[PATHSZ];
	char	cmd[PATHSZ * 3];

	xsnprintf(path, sizeof(path), "%s/%s/%s", repo, ex->dir, ex->src2);
	if (access(path, R_OK) != 0 || !abs_path(path, out, n))
	{
		printf("  %s[KO]%s falta %s, que el subject pide junto a %s\n",
			C_KO, C_0, ex->src2, ex->src);
		return (0);
	}
	xsnprintf(cmd, sizeof(cmd),
		"cc -Wall -Wextra -Werror -c -o '%s/%s2.o' '%s' > '%s' 2>&1",
		g_tmp, ex->dir, out, log);
	if (system(cmd) != 0)
	{
		printf("  %s[COMPILA KO]%s %s con -Wall -Wextra -Werror\n",
			C_KO, C_0, ex->src2);
		dump_log(log);
		return (0);
	}
	return (1);
}

/* ========================================================================== */
/*                      COMPROBACIONES DE SHELL (ex00-ex05)                   */
/* ========================================================================== */
/* Ninguna toca el repo del alumno: lo que haga falta ejecutar (clean,         */
/* find_sh.sh) corre sobre una copia en un arbol de prueba dentro de /tmp,     */
/* que es justo lo que interesa probar y ademas no se lleva nada por delante.  */

static int	g_sko;

static void	s_detail(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));

static void	s_detail(const char *fmt, ...)
{
	va_list	ap;

	printf("         %s", C_D);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("%s\n", C_0);
}

static void	s_res(int pass, const char *fmt, ...)
	__attribute__((format(printf, 2, 3)));

static void	s_res(int pass, const char *fmt, ...)
{
	va_list	ap;

	if (pass)
		printf("  %s[OK]%s   ", C_OK, C_0);
	else
	{
		g_sko++;
		printf("  %s[KO]%s   ", C_KO, C_0);
	}
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
}

/* Version de una linea de un texto, para meterlo en un detalle. */
static const char	*sq(const char *s)
{
	static char	buf[2][240];
	static int	turn;
	char		*d;
	size_t		k;

	d = buf[turn];
	turn = (turn + 1) % 2;
	if (!s)
		return ("(nada)");
	k = 0;
	d[k++] = '"';
	while (*s && k < 230)
	{
		if (*s == '\n')
			k += (size_t)sprintf(d + k, "\\n");
		else if (*s == '\t')
			k += (size_t)sprintf(d + k, "\\t");
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

/* Arbol de pruebas vacio para el ejercicio (se rehace en cada pasada). */
static int	work_dir(const char *name, char *out, size_t n)
{
	char	cmd[PATHSZ * 2];

	xsnprintf(out, n, "%s/w_%s", g_tmp, name);
	xsnprintf(cmd, sizeof(cmd),
		"chmod -R u+rwX '%s' 2>/dev/null; rm -rf '%s' && mkdir -p '%s'",
		out, out, out);
	return (system(cmd) == 0);
}

/* Ejecuta cmd con sh dentro de dir y devuelve su salida (stdout+stderr). */
static char	*sh_run(const char *dir, const char *cmd)
{
	char	out[PATHSZ];
	char	full[PATHSZ * 4];

	xsnprintf(out, sizeof(out), "%s/sh.out", g_tmp);
	xsnprintf(full, sizeof(full), "cd '%s' && { %s ; } > '%s' 2>&1",
		dir, cmd, out);
	if (system(full) == -1)
		return (NULL);
	return (slurp(out));
}

static int	sh_do(const char *dir, const char *cmd)
{
	char	full[PATHSZ * 4];

	xsnprintf(full, sizeof(full), "cd '%s' && { %s ; } >/dev/null 2>&1",
		dir, cmd);
	return (system(full) == 0);
}

static void	modestr(mode_t m, char *out)
{
	const char	*rwx = "rwxrwxrwx";
	int			i;

	if (S_ISDIR(m))
		out[0] = 'd';
	else if (S_ISLNK(m))
		out[0] = 'l';
	else
		out[0] = '-';
	i = 0;
	while (i < 9)
	{
		out[i + 1] = (m & (1 << (8 - i))) ? rwx[i] : '-';
		i++;
	}
	out[10] = '\0';
}

static int	exists(const char *dir, const char *name)
{
	char		path[PATHSZ];
	struct stat	st;

	xsnprintf(path, sizeof(path), "%s/%s", dir, name);
	return (lstat(path, &st) == 0);
}

/* ------------------------------- ex00 ------------------------------------ */
/* El tar tiene que traer exactamente el ls -l del subject: siete entradas,   */
/* con sus permisos, sus tamanos, un enlace duro y uno simbolico.             */

typedef struct s_tarent
{
	const char	*name;
	const char	*mode;
	long		size;
	int			hh;
	int			mm;
}	t_tarent;

static const t_tarent	g_tar[] = {
{"test0", "drwx--xr-x", -1, 20, 47},
{"test1", "-rwx--xr--", 4, 21, 46},
{"test2", "dr-x---r--", -1, 22, 45},
{"test3", "-r-----r--", 1, 23, 44},
{"test4", "-rw-r----x", 2, 23, 43},
{"test5", "-r-----r--", 1, 23, 44},
{"test6", "lrwxrwxrwx", -1, 22, 20},
};
# define N_TAR (sizeof(g_tar) / sizeof(g_tar[0]))

static void	ck_tar_entry(const char *dir, const t_tarent *e)
{
	char		path[PATHSZ];
	char		mode[11];
	struct stat	st;
	struct tm	*tm;

	xsnprintf(path, sizeof(path), "%s/%s", dir, e->name);
	if (lstat(path, &st) != 0)
		return ((void)s_res(0, "%s esta en el tar", e->name));
	modestr(st.st_mode, mode);
	if (strcmp(mode, e->mode) != 0)
	{
		s_res(0, "%s es %s", e->name, e->mode);
		s_detail("obtenido %s", mode);
		return ;
	}
	if (e->size >= 0 && st.st_size != (off_t)e->size)
	{
		s_res(0, "%s ocupa %ld bytes", e->name, e->size);
		s_detail("obtenido %ld", (long)st.st_size);
		return ;
	}
	tm = localtime(&st.st_mtime);
	if (!tm || tm->tm_mon != 5 || tm->tm_mday != 1
		|| tm->tm_hour != e->hh || tm->tm_min != e->mm)
	{
		s_res(0, "%s tiene la fecha del subject (1 jun %02d:%02d)",
			e->name, e->hh, e->mm);
		if (tm)
			s_detail("obtenido %d %s %02d:%02d (ojo a la zona horaria)",
				tm->tm_mday, tm->tm_mon == 5 ? "jun" : "otro mes",
				tm->tm_hour, tm->tm_min);
		return ;
	}
	s_res(1, "%s: %s%s%s", e->name, mode,
		e->size >= 0 ? ", " : "", e->size >= 0 ? "tamano y fecha ok" : "");
}

static void	ck_tar_links(const char *dir)
{
	char		p3[PATHSZ];
	char		p5[PATHSZ];
	char		p6[PATHSZ];
	char		tgt[PATHSZ];
	struct stat	s3;
	struct stat	s5;
	ssize_t		n;

	xsnprintf(p3, sizeof(p3), "%s/test3", dir);
	xsnprintf(p5, sizeof(p5), "%s/test5", dir);
	xsnprintf(p6, sizeof(p6), "%s/test6", dir);
	if (lstat(p3, &s3) == 0 && lstat(p5, &s5) == 0)
	{
		if (s3.st_ino == s5.st_ino && s5.st_nlink >= 2)
			s_res(1, "test5 es un enlace duro a test3");
		else
		{
			s_res(0, "test5 es un enlace duro a test3");
			s_detail("inodos %lu y %lu, %lu enlaces",
				(unsigned long)s3.st_ino, (unsigned long)s5.st_ino,
				(unsigned long)s5.st_nlink);
		}
	}
	else
		s_res(0, "test5 es un enlace duro a test3");
	n = readlink(p6, tgt, sizeof(tgt) - 1);
	if (n < 0)
		return ((void)s_res(0, "test6 apunta a test0"));
	tgt[n] = '\0';
	s_res(strcmp(tgt, "test0") == 0, "test6 apunta a test0");
	if (strcmp(tgt, "test0") != 0)
		s_detail("apunta a %s", sq(tgt));
}

static int	ex_tar(const char *src)
{
	char	dir[PATHSZ];
	char	cmd[PATHSZ * 3];
	size_t	i;

	if (!work_dir("tar", dir, sizeof(dir)))
		return (-1);
	xsnprintf(cmd, sizeof(cmd), "tar -xpf '%s' -C '%s' 2>&1", src, dir);
	if (system(cmd) != 0)
	{
		s_res(0, "exo.tar se extrae con tar -xf");
		return (g_sko);
	}
	s_res(1, "exo.tar se extrae con tar -xf");
	i = 0;
	while (i < N_TAR)
		ck_tar_entry(dir, &g_tar[i++]);
	ck_tar_links(dir);
	return (g_sko);
}

/* ------------------------------- ex01 ------------------------------------ */

static int	ex_z(const char *src)
{
	char	*txt;

	txt = slurp(src);
	if (!txt)
		return (-1);
	s_res(strcmp(txt, "Z\n") == 0, "cat z muestra \"Z\" y un salto de linea");
	if (strcmp(txt, "Z\n") != 0)
		s_detail("obtenido %s", sq(txt));
	free(txt);
	return (g_sko);
}

/* ------------------------------- ex02 ------------------------------------ */
/* Un solo comando (nada de ';' ni '&&'), que ademas muestre lo que borra.    */

static const char	*g_clean_kill[] = {"a~", "#b#", "sub/c~", "sub/#d#"};
static const char	*g_clean_keep[] = {"keep", "~lead", "#only", "only#",
	"sub/keep.txt"};

static void	ck_clean_oneliner(const char *txt)
{
	const char	*bad;

	bad = NULL;
	if (strstr(txt, "&&"))
		bad = "&&";
	else if (strstr(txt, "||"))
		bad = "||";
	else if (strchr(txt, ';'))
		bad = ";";
	s_res(bad == NULL, "un solo comando (sin ';' ni '&&')");
	if (bad)
		s_detail("encontrado %s en la linea", bad);
}

static int	ex_clean(const char *src)
{
	char	dir[PATHSZ];
	char	cmd[PATHSZ * 3];
	char	*txt;
	char	*out;
	size_t	i;

	txt = slurp(src);
	if (!txt)
		return (-1);
	ck_clean_oneliner(txt);
	free(txt);
	if (!work_dir("clean", dir, sizeof(dir)))
		return (-1);
	if (!sh_do(dir, "mkdir sub && touch 'a~' '#b#' keep '~lead' '#only' "
			"'only#' 'sub/c~' 'sub/#d#' sub/keep.txt"))
		return (-1);
	xsnprintf(cmd, sizeof(cmd), "cp '%s' '%s/clean'", src, dir);
	if (system(cmd) != 0)
		return (-1);
	out = sh_run(dir, "sh clean");
	i = 0;
	while (i < sizeof(g_clean_kill) / sizeof(*g_clean_kill))
	{
		s_res(!exists(dir, g_clean_kill[i]), "borra %s", g_clean_kill[i]);
		if (out && !strstr(out, g_clean_kill[i]))
			s_detail("ademas tiene que mostrarlo: no sale en la salida");
		i++;
	}
	i = 0;
	while (i < sizeof(g_clean_keep) / sizeof(*g_clean_keep))
	{
		s_res(exists(dir, g_clean_keep[i]), "respeta %s", g_clean_keep[i]);
		i++;
	}
	s_res(out && *out, "muestra lo que borra");
	free(out);
	return (g_sko);
}

/* ------------------------------- ex03 ------------------------------------ */
/* Los .sh del arbol, sin ruta y sin extension. El orden no importa: se       */
/* comparan como conjunto, porque find no promete uno.                        */

static int	cmp_str(const void *a, const void *b)
{
	return (strcmp(*(const char **)a, *(const char **)b));
}

/* El motivo se deja en why en vez de imprimirlo: el panel engancha los       */
/* detalles a la linea [KO] anterior, asi que tienen que salir despues.       */
static int	ck_lines(char *out, const char **want, size_t nwant, char *why)
{
	char	*got[64];
	size_t	n;
	size_t	i;
	char	*tok;

	n = 0;
	tok = strtok(out, "\n");
	while (tok && n < 64)
	{
		if (*tok)
			got[n++] = tok;
		tok = strtok(NULL, "\n");
	}
	qsort(got, n, sizeof(char *), cmp_str);
	if (n != nwant)
	{
		sprintf(why, "esperadas %zu lineas, obtenidas %zu", nwant, n);
		return (0);
	}
	i = 0;
	while (i < n)
	{
		if (strcmp(got[i], want[i]) != 0)
		{
			sprintf(why, "esperado %s, obtenido %s", sq(want[i]), sq(got[i]));
			return (0);
		}
		i++;
	}
	return (1);
}

static int	ex_find_sh(const char *src)
{
	static const char	*want[] = {"file1", "file2", "file3", "find_sh"};
	char				dir[PATHSZ];
	char				cmd[PATHSZ * 3];
	char				why[512];
	char				*out;
	int					ok;

	if (!work_dir("findsh", dir, sizeof(dir)))
		return (-1);
	if (!sh_do(dir, "mkdir -p sub/deep && touch file1.sh sub/file2.sh "
			"sub/deep/file3.sh nope.shx sh x.sh.txt"))
		return (-1);
	xsnprintf(cmd, sizeof(cmd), "cp '%s' '%s/find_sh.sh'", src, dir);
	if (system(cmd) != 0)
		return (-1);
	out = sh_run(dir, "sh find_sh.sh");
	if (!out)
		return (-1);
	why[0] = '\0';
	ok = ck_lines(out, want, 4, why);
	s_res(ok, "lista file1 file2 file3 find_sh (sin ruta y sin .sh)");
	if (!ok && why[0])
		s_detail("%s", why);
	free(out);
	return (g_sko);
}

/* ------------------------------- ex04 ------------------------------------ */

static int	is_mac(const char *s)
{
	int	i;

	i = 0;
	while (i < 17)
	{
		if ((i % 3) == 2)
		{
			if (s[i] != ':')
				return (0);
		}
		else if (!isxdigit((unsigned char)s[i]))
			return (0);
		i++;
	}
	return (s[17] == '\0');
}

static int	ex_mac(const char *src)
{
	char	dir[PATHSZ];
	char	cmd[PATHSZ * 3];
	char	*out;
	char	*tok;
	int		n;
	int		bad;

	if (!work_dir("mac", dir, sizeof(dir)))
		return (-1);
	xsnprintf(cmd, sizeof(cmd), "cp '%s' '%s/MAC.sh'", src, dir);
	if (system(cmd) != 0)
		return (-1);
	out = sh_run(dir, "sh MAC.sh");
	if (!out)
		return (-1);
	n = 0;
	bad = 0;
	tok = strtok(out, "\n");
	while (tok)
	{
		if (*tok && !is_mac(tok))
			bad++;
		else if (*tok)
			n++;
		tok = strtok(NULL, "\n");
	}
	if (n == 0 && system("command -v ifconfig >/dev/null 2>&1") != 0)
		printf("  %s[AVISO]%s aqui no hay ifconfig, asi que esto no prueba "
			"nada: en el cluster si lo hay\n", C_SK, C_0);
	else
	{
		s_res(n > 0, "muestra al menos una MAC");
		s_res(bad == 0, "todas las lineas son una MAC y nada mas");
		if (bad)
			s_detail("%d linea(s) no tienen forma de MAC", bad);
	}
	free(out);
	return (g_sko);
}

/* ------------------------------- ex05 ------------------------------------ */

static int	ex_marvin(const char *src)
{
	char	*txt;

	txt = slurp(src);
	if (!txt)
		return (-1);
	s_res(strcmp(txt, "42") == 0, "el archivo contiene 42 y nada mas");
	if (strcmp(txt, "42") != 0)
		s_detail("obtenido %s (tiene que ocupar 2 bytes, sin salto final)",
			sq(txt));
	free(txt);
	return (g_sko);
}

/* -------------------------------------------------------------------------- */

static int	do_shell_ex(const char *repo, const t_ex *ex)
{
	char	src[PATHSZ];
	int		r;

	xsnprintf(src, sizeof(src), "%s/%s/%s", repo, ex->dir, ex->src);
	if (access(src, R_OK) != 0)
	{
		printf("  %s[SKIP]%s archivo no encontrado (%s)\n", C_SK, C_0, src);
		return (-2);
	}
	g_sko = 0;
	r = -1;
	if (ex->id == 0)
		r = ex_tar(src);
	else if (ex->id == 1)
		r = ex_z(src);
	else if (ex->id == 2)
		r = ex_clean(src);
	else if (ex->id == 3)
		r = ex_find_sh(src);
	else if (ex->id == 4)
		r = ex_mac(src);
	else if (ex->id == 5)
		r = ex_marvin(src);
	if (r < 0)
		printf("  %s[TEST KO]%s no se pudo montar el arbol de prueba\n",
			C_KO, C_0);
	return (r);
}

/* ========================================================================== */
/*                  PROGRAMAS, CABECERAS Y MAKEFILES (ex18-ex27)              */
/* ========================================================================== */
/* Lo que aqui se prueba no es una funcion suelta: ex18 y ex19 son programas  */
/* con su main, ex22 y ex23 son cabeceras (no hay nada que ejecutar hasta que */
/* no les escribes un main alrededor) y ex24 y ex27 son Makefiles, que se     */
/* prueban con fuentes de mentira como hace la moulinette.                    */

static int	write_file(const char *path, const char *content)
{
	FILE	*f;

	f = fopen(path, "w");
	if (!f)
		return (0);
	fputs(content, f);
	return (fclose(f) == 0);
}

/* Compila fuentes sueltos con los flags de la moulinette. 0 = no compila. */
static int	build(const char *cmd, const char *log)
{
	char	full[PATHSZ * 4];

	xsnprintf(full, sizeof(full), "%s > '%s' 2>&1", cmd, log);
	return (system(full) == 0);
}

/* Ejecuta cmd dentro de dir separando salida y errores. */
static void	run_2(const char *dir, const char *cmd, char **out, char **err)
{
	char	po[PATHSZ];
	char	pe[PATHSZ];
	char	full[PATHSZ * 4];

	xsnprintf(po, sizeof(po), "%s/o.txt", g_tmp);
	xsnprintf(pe, sizeof(pe), "%s/e.txt", g_tmp);
	xsnprintf(full, sizeof(full), "cd '%s' && { %s ; } > '%s' 2> '%s'",
		dir, cmd, po, pe);
	if (system(full) == -1)
		fprintf(stderr, "aviso: no se pudo ejecutar %s\n", cmd);
	*out = slurp(po);
	*err = slurp(pe);
}

static void	ck_out(const char *got, const char *exp, const char *label)
{
	if (got && strcmp(got, exp) == 0)
		return ((void)s_res(1, "%s", label));
	s_res(0, "%s", label);
	s_detail("esperado %s", sq(exp));
	s_detail("obtenido %s", sq(got));
}

/* ---------------------------- ex18 y ex19 -------------------------------- */

static void	argv_label(char **argv, char *out, size_t n)
{
	size_t	k;
	int		i;

	k = 0;
	i = 1;
	out[0] = '\0';
	k += (size_t)snprintf(out + k, n - k, "./a.out");
	while (argv[i] && k + 4 < n)
	{
		k += (size_t)snprintf(out + k, n - k, " \"%s\"", argv[i]);
		i++;
	}
}

/* Lanza el programa del alumno con un argv concreto y devuelve su stdout.   */
static char	*run_argv(const char *bin, char **argv, int *sig)
{
	int		fds[2];
	int		status;
	pid_t	pid;
	char	*out;

	*sig = 0;
	if (pipe(fds) == -1)
		return (NULL);
	pid = fork();
	if (pid == -1)
		return (close(fds[0]), close(fds[1]), NULL);
	if (pid == 0)
	{
		dup2(fds[1], STDOUT_FILENO);
		close(fds[0]);
		close(fds[1]);
		alarm(TIMEOUT);
		execv(bin, argv);
		_exit(127);
	}
	close(fds[1]);
	out = read_all(fds[0]);
	close(fds[0]);
	if (waitpid(pid, &status, 0) != -1 && WIFSIGNALED(status))
		*sig = WTERMSIG(status);
	return (out);
}

static void	prog_case(const char *bin, char **argv, const char *exp)
{
	char	label[256];
	char	*out;
	int		sig;

	argv_label(argv, label, sizeof(label));
	out = run_argv(bin, argv, &sig);
	if (sig)
	{
		s_res(0, "%s", label);
		s_detail("el programa muere con senal %d (%s)", sig, strsignal(sig));
	}
	else
		ck_out(out, exp, label);
	free(out);
}

static void	cases_print_params(const char *bin)
{
	static char	*a1[] = {"./ft_print_params", "test1", "test2", "test3", NULL};
	static char	*a2[] = {"./ft_print_params", NULL};
	static char	*a3[] = {"./ft_print_params", "uno", NULL};
	static char	*a4[] = {"./ft_print_params", "", "x", NULL};

	prog_case(bin, a1, "test1\ntest2\ntest3\n");
	prog_case(bin, a3, "uno\n");
	prog_case(bin, a2, "");
	prog_case(bin, a4, "\nx\n");
}

static void	cases_sort_params(const char *bin)
{
	static char	*a1[] = {"./ft_sort_params", "z", "a", "m", NULL};
	static char	*a2[] = {"./ft_sort_params", NULL};
	static char	*a3[] = {"./ft_sort_params", "b", "B", "a", "A", NULL};
	static char	*a4[] = {"./ft_sort_params", "42", "42", "1", NULL};
	static char	*a5[] = {"./ft_sort_params", "solo", NULL};

	prog_case(bin, a1, "a\nm\nz\n");
	prog_case(bin, a3, "A\nB\na\nb\n");
	prog_case(bin, a4, "1\n42\n42\n");
	prog_case(bin, a5, "solo\n");
	prog_case(bin, a2, "");
}

static int	do_prog_ex(const char *repo, const t_ex *ex)
{
	char	src[PATHSZ];
	char	bin[PATHSZ];
	char	log[PATHSZ];
	char	cmd[PATHSZ * 3];

	xsnprintf(src, sizeof(src), "%s/%s/%s", repo, ex->dir, ex->src);
	if (access(src, R_OK) != 0)
	{
		printf("  %s[SKIP]%s fuente no encontrado (%s)\n", C_SK, C_0, src);
		hint_other_sources(repo, ex);
		return (-2);
	}
	xsnprintf(log, sizeof(log), "%s/%s.log", g_tmp, ex->dir);
	xsnprintf(bin, sizeof(bin), "%s/%s.bin", g_tmp, ex->dir);
	xsnprintf(cmd, sizeof(cmd), "cc -Wall -Wextra -Werror -o '%s' '%s'",
		bin, src);
	if (!build(cmd, log))
	{
		printf("  %s[COMPILA KO]%s con -Wall -Wextra -Werror\n", C_KO, C_0);
		dump_log(log);
		return (-1);
	}
	g_sko = 0;
	if (ex->id == 18)
		cases_print_params(bin);
	else
		cases_sort_params(bin);
	return (g_sko);
}

/* ---------------------------- ex22 y ex23 -------------------------------- */
/* Un probe es un main de mentira alrededor de la cabecera del alumno: es la  */
/* unica forma de saber si ABS esta bien puesta entre parentesis o si t_point */
/* tiene los campos que el subject usa.                                       */

static int	probe(const char *dir, const char *hdr, const char *body,
		const char *label)
{
	char	code[PATHSZ * 2];
	char	cpath[PATHSZ];
	char	bin[PATHSZ];
	char	log[PATHSZ];
	char	cmd[PATHSZ * 3];

	xsnprintf(code, sizeof(code), "#include \"%s\"\n%s\n", hdr, body);
	xsnprintf(cpath, sizeof(cpath), "%s/probe.c", g_tmp);
	xsnprintf(bin, sizeof(bin), "%s/probe.bin", g_tmp);
	xsnprintf(log, sizeof(log), "%s/probe.log", g_tmp);
	if (!write_file(cpath, code))
		return (0);
	xsnprintf(cmd, sizeof(cmd),
		"cc -Wall -Wextra -Werror -I '%s' -o '%s' '%s'", dir, bin, cpath);
	if (!build(cmd, log))
	{
		s_res(0, "%s", label);
		dump_log(log);
		return (0);
	}
	s_res(1, "%s", label);
	return (1);
}

/* Igual, pero ademas ejecuta y compara la salida. */
static void	probe_out(const char *dir, const char *hdr, const char *body,
		const char *exp, const char *label)
{
	char	bin[PATHSZ];
	char	*out;
	char	*err;

	if (!probe(dir, hdr, body, label))
		return ;
	xsnprintf(bin, sizeof(bin), "%s/probe.bin", g_tmp);
	run_2(g_tmp, "./probe.bin", &out, &err);
	ck_out(out, exp, "y da el resultado esperado");
	free(out);
	free(err);
}

static void	cases_abs(const char *dir)
{
	probe(dir, "ft_abs.h", "int\tmain(void)\n{\n\treturn (ABS(-1) == 1);\n}",
		"ft_abs.h define ABS y compila");
	probe_out(dir, "ft_abs.h",
		"#include <stdio.h>\n"
		"int\tmain(void)\n"
		"{\n"
		"\tint\tv;\n"
		"\n"
		"\tv = -7;\n"
		"\tprintf(\"%d %d %d %d %d\\n\", ABS(-5), ABS(5), ABS(0), ABS(3 - 8), ABS(v));\n"
		"\treturn (0);\n"
		"}",
		"5 5 0 5 7\n",
		"ABS(x) vale para negativos, positivos y expresiones");
	probe(dir, "ft_abs.h",
		"#include \"ft_abs.h\"\n"
		"int\tmain(void)\n{\n\treturn (ABS(-1) == 1);\n}",
		"se puede incluir dos veces (tiene guardas)");
}

static void	cases_point(const char *dir)
{
	probe(dir, "ft_point.h",
		"void\tset_point(t_point *point)\n"
		"{\n"
		"\tpoint->x = 42;\n"
		"\tpoint->y = 21;\n"
		"}\n"
		"\n"
		"int\tmain(void)\n"
		"{\n"
		"\tt_point\tpoint;\n"
		"\n"
		"\tset_point(&point);\n"
		"\treturn (0);\n"
		"}",
		"compila el main del subject tal cual");
	probe_out(dir, "ft_point.h",
		"#include <stdio.h>\n"
		"int\tmain(void)\n"
		"{\n"
		"\tt_point\tp;\n"
		"\n"
		"\tp.x = 42;\n"
		"\tp.y = 21;\n"
		"\tprintf(\"%d %d\\n\", p.x, p.y);\n"
		"\treturn (0);\n"
		"}",
		"42 21\n",
		"t_point tiene x e y enteros");
	probe(dir, "ft_point.h",
		"#include \"ft_point.h\"\n"
		"int\tmain(void)\n{\n\tt_point\tp;\n\n\tp.x = 0;\n\treturn (p.x);\n}",
		"se puede incluir dos veces (tiene guardas)");
}

static int	do_hdr_ex(const char *repo, const t_ex *ex)
{
	char	dir[PATHSZ];
	char	src[PATHSZ];
	char	abs[PATHSZ];

	xsnprintf(src, sizeof(src), "%s/%s/%s", repo, ex->dir, ex->src);
	if (access(src, R_OK) != 0)
	{
		printf("  %s[SKIP]%s cabecera no encontrada (%s)\n", C_SK, C_0, src);
		return (-2);
	}
	xsnprintf(dir, sizeof(dir), "%s/%s", repo, ex->dir);
	if (!abs_path(dir, abs, sizeof(abs)))
		return (-1);
	g_sko = 0;
	if (ex->id == 22)
		cases_abs(abs);
	else
		cases_point(abs);
	return (g_sko);
}

/* ------------------------------- ex24 ------------------------------------ */
/* La moulinette prueba el Makefile con SUS fuentes, no con los del alumno,   */
/* asi que aqui se monta un proyecto de mentira con las cinco funciones que   */
/* el subject nombra y se mira que las reglas hagan lo que prometen.          */

static const char	*g_libft_h =
	"#ifndef LIBFT_H\n# define LIBFT_H\n\n# include <unistd.h>\n\n"
	"void\tft_putchar(char c);\nvoid\tft_putstr(char *str);\n"
	"int\t\tft_strcmp(char *s1, char *s2);\nint\t\tft_strlen(char *str);\n"
	"void\tft_swap(int *a, int *b);\n\n#endif\n";

static int	make_fake_lib(const char *dir)
{
	char	path[PATHSZ];
	int		ok;

	xsnprintf(path, sizeof(path), "mkdir -p '%s/srcs' '%s/includes' '%s/check'",
		dir, dir, dir);
	if (system(path) != 0)
		return (0);
	xsnprintf(path, sizeof(path), "%s/includes/libft.h", dir);
	ok = write_file(path, g_libft_h);
	xsnprintf(path, sizeof(path), "%s/srcs/ft_putchar.c", dir);
	ok &= write_file(path, "#include \"libft.h\"\n\nvoid\tft_putchar(char c)\n"
			"{\n\twrite(1, &c, 1);\n}\n");
	xsnprintf(path, sizeof(path), "%s/srcs/ft_putstr.c", dir);
	ok &= write_file(path, "#include \"libft.h\"\n\nvoid\tft_putstr(char *str)\n"
			"{\n\twhile (*str)\n\t\tft_putchar(*str++);\n}\n");
	xsnprintf(path, sizeof(path), "%s/srcs/ft_strlen.c", dir);
	ok &= write_file(path, "#include \"libft.h\"\n\nint\tft_strlen(char *str)\n"
			"{\n\tint\ti;\n\n\ti = 0;\n\twhile (str[i])\n\t\ti++;\n"
			"\treturn (i);\n}\n");
	xsnprintf(path, sizeof(path), "%s/srcs/ft_strcmp.c", dir);
	ok &= write_file(path, "#include \"libft.h\"\n\nint\tft_strcmp(char *s1, "
			"char *s2)\n{\n\twhile (*s1 && *s1 == *s2)\n\t{\n\t\ts1++;\n"
			"\t\ts2++;\n\t}\n\treturn (*s1 - *s2);\n}\n");
	xsnprintf(path, sizeof(path), "%s/srcs/ft_swap.c", dir);
	ok &= write_file(path, "#include \"libft.h\"\n\nvoid\tft_swap(int *a, "
			"int *b)\n{\n\tint\tt;\n\n\tt = *a;\n\t*a = *b;\n\t*b = t;\n}\n");
	xsnprintf(path, sizeof(path), "%s/check/main.c", dir);
	ok &= write_file(path, "#include \"libft.h\"\n\nint\tmain(void)\n{\n"
			"\tint\ta;\n\tint\tb;\n\n\ta = 1;\n\tb = 2;\n\tft_swap(&a, &b);\n"
			"\tft_putstr(\"ok\");\n\tft_putchar('\\n');\n"
			"\tif (a != 2 || ft_strlen(\"42\") != 2 || ft_strcmp(\"a\", \"a\"))\n"
			"\t\treturn (1);\n\treturn (0);\n}\n");
	return (ok);
}

static int	count_objs(const char *dir)
{
	char	cmd[PATHSZ * 2];
	char	out[PATHSZ];
	char	*txt;
	int		n;

	xsnprintf(out, sizeof(out), "%s/objs.txt", g_tmp);
	xsnprintf(cmd, sizeof(cmd),
		"find '%s' -name '*.o' | wc -l > '%s' 2>/dev/null", dir, out);
	if (system(cmd) != 0)
		return (-1);
	txt = slurp(out);
	n = txt ? atoi(txt) : -1;
	free(txt);
	return (n);
}

static void	ck_lib_links(const char *dir)
{
	char	cmd[PATHSZ * 3];
	char	log[PATHSZ];
	char	*out;
	char	*err;

	xsnprintf(log, sizeof(log), "%s/link.log", g_tmp);
	xsnprintf(cmd, sizeof(cmd), "cd '%s' && cc -Wall -Wextra -Werror "
		"-Iincludes -o check/prog check/main.c libft.a", dir);
	if (!build(cmd, log))
	{
		s_res(0, "la libft.a enlaza y sus funciones responden");
		dump_log(log);
		return ;
	}
	run_2(dir, "./check/prog", &out, &err);
	ck_out(out, "ok\n", "la libft.a enlaza y sus funciones responden");
	free(out);
	free(err);
}

static int	ex_makefile_lib(const char *repo, const t_ex *ex)
{
	char	dir[PATHSZ];
	char	cmd[PATHSZ * 3];
	char	lib[PATHSZ];
	char	*out;
	char	*err;
	int		objs;

	if (!work_dir("make24", dir, sizeof(dir)) || !make_fake_lib(dir))
		return (-1);
	xsnprintf(cmd, sizeof(cmd), "cp '%s/%s/Makefile' '%s/Makefile'",
		repo, ex->dir, dir);
	if (system(cmd) != 0)
		return (-1);
	xsnprintf(lib, sizeof(lib), "%s/libft.a", dir);
	run_2(dir, "make", &out, &err);
	free(out);
	free(err);
	s_res(access(lib, F_OK) == 0, "make deja libft.a en la raiz");
	if (access(lib, F_OK) != 0)
		return (g_sko);
	run_2(dir, "ar -t libft.a | sort | tr '\\n' ' '", &out, &err);
	ck_out(out, "ft_putchar.o ft_putstr.o ft_strcmp.o ft_strlen.o ft_swap.o ",
		"la libreria trae los cinco objetos");
	free(out);
	free(err);
	ck_lib_links(dir);
	run_2(dir, "make clean", &out, &err);
	free(out);
	free(err);
	objs = count_objs(dir);
	s_res(objs == 0, "make clean borra los .o");
	if (objs > 0)
		s_detail("quedan %d fichero(s) .o", objs);
	s_res(access(lib, F_OK) == 0, "make clean respeta la libreria");
	run_2(dir, "make fclean", &out, &err);
	free(out);
	free(err);
	s_res(access(lib, F_OK) != 0, "make fclean borra la libreria");
	run_2(dir, "make re", &out, &err);
	free(out);
	free(err);
	s_res(access(lib, F_OK) == 0, "make re la vuelve a dejar");
	return (g_sko);
}

/* ------------------------------- ex27 ------------------------------------ */

static void	ck_display_errors(const char *dir)
{
	char	*out;
	char	*err;

	run_2(dir, "./ft_display_file", &out, &err);
	ck_out(err, "File name missing.\n", "sin argumentos avisa por stderr");
	if (out && *out)
		s_detail("ademas escribio %s en la salida estandar", sq(out));
	free(out);
	free(err);
	run_2(dir, "./ft_display_file hola.txt otro.txt", &out, &err);
	ck_out(err, "Too many arguments.\n", "con dos argumentos avisa por stderr");
	free(out);
	free(err);
	run_2(dir, "./ft_display_file no_existe.txt", &out, &err);
	ck_out(err, "Cannot read file.\n", "con un archivo ilegible avisa por stderr");
	free(out);
	free(err);
}

static int	ex_makefile_display(const char *repo, const t_ex *ex)
{
	char	dir[PATHSZ];
	char	cmd[PATHSZ * 3];
	char	bin[PATHSZ];
	char	path[PATHSZ];
	char	*out;
	char	*err;

	if (!work_dir("make27", dir, sizeof(dir)))
		return (-1);
	xsnprintf(cmd, sizeof(cmd), "cp -r '%s/%s/.' '%s/'", repo, ex->dir, dir);
	if (system(cmd) != 0)
		return (-1);
	xsnprintf(path, sizeof(path), "%s/hola.txt", dir);
	if (!write_file(path, "42\nes la respuesta\n"))
		return (-1);
	xsnprintf(bin, sizeof(bin), "%s/ft_display_file", dir);
	run_2(dir, "make", &out, &err);
	free(out);
	free(err);
	if (access(bin, X_OK) != 0)
	{
		s_res(0, "make deja el binario ft_display_file");
		return (g_sko);
	}
	s_res(1, "make deja el binario ft_display_file");
	run_2(dir, "./ft_display_file hola.txt", &out, &err);
	ck_out(out, "42\nes la respuesta\n", "muestra el contenido del archivo");
	if (err && *err)
		s_detail("ademas escribio %s en stderr", sq(err));
	free(out);
	free(err);
	ck_display_errors(dir);
	run_2(dir, "make fclean", &out, &err);
	free(out);
	free(err);
	s_res(access(bin, F_OK) != 0, "make fclean borra el binario");
	return (g_sko);
}

static int	do_make_ex(const char *repo, const t_ex *ex)
{
	char	src[PATHSZ];

	xsnprintf(src, sizeof(src), "%s/%s/%s", repo, ex->dir, ex->src);
	if (access(src, R_OK) != 0)
	{
		printf("  %s[SKIP]%s Makefile no encontrado (%s)\n", C_SK, C_0, src);
		return (-2);
	}
	g_sko = 0;
	if (ex->id == 24)
		return (ex_makefile_lib(repo, ex));
	return (ex_makefile_display(repo, ex));
}

/* Devuelve el numero de tests KO, -1 si el ejercicio no se pudo probar,      */
/* -2 si el fuente no existe (SKIP).                                          */
static int	do_ex(const char *repo, const char *self, const t_ex *ex)
{
	char	src[PATHSZ];
	char	abs[PATHSZ];
	char	abs2[PATHSZ];
	char	def2[PATHSZ];
	char	obj[PATHSZ];
	char	bin[PATHSZ];
	char	log[PATHSZ];
	char	casef[PATHSZ];
	char	cmd[PATHSZ * 4];

	printf("%s── %s/%s%s\n", C_B, ex->dir, ex->src, C_0);
	if (ex->kind == EX_SHELL)
		return (do_shell_ex(repo, ex));
	if (ex->kind == EX_PROG)
		return (do_prog_ex(repo, ex));
	if (ex->kind == EX_HDR)
		return (do_hdr_ex(repo, ex));
	if (ex->kind == EX_MAKE)
		return (do_make_ex(repo, ex));
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
	def2[0] = '\0';
	if (ex->src2 && !second_source(repo, ex, abs2, sizeof(abs2), log))
		return (-1);
	if (ex->src2)
		xsnprintf(def2, sizeof(def2), "-DFT_SRC2='\"%s\"' ", abs2);
	/* -Wno-return-type: al renombrar main() deja de aplicarsele el trato
	   especial del estandar y "cae" sin return; ya se comprobo intacto arriba */
	xsnprintf(cmd, sizeof(cmd),
		"cc -Wall -Wextra -Werror -Wno-return-type -DFT_EX=%d -DFT_SRC='\"%s\"' "
		"%s-Dmain=ft_test_unused_main -o '%s' '%s' > '%s' 2>&1",
		ex->id, abs, def2, bin, self, log);
	if (system(cmd) != 0)
	{
		printf("  %s[TEST KO]%s no se pudo montar el test (prototipo distinto "
			"del que pide el subject?)\n", C_KO, C_0);
		dump_log(log);
		return (-1);
	}
	xsnprintf(casef, sizeof(casef), "%s/%s.case", g_tmp, ex->dir);
	return (run_bin(bin, casef));
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
	strcpy(g_tmp, "/tmp/reloadedtest.XXXXXX");
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
