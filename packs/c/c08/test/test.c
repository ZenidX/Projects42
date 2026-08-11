/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para los ejercicios de C08                      */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*              (o desde test/:  make)                                        */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*              (o desde test/:  make test)                                   */
/*                                                                            */
/*   C08 se sale de la plantilla de los packs anteriores porque lo que se      */
/*   entrega casi siempre es una CABECERA, no una funcion: no hay nada que     */
/*   ejecutar, lo que se prueba es si compila lo que el subject dice que tiene */
/*   que compilar. De ahi que haya dos clases de comprobacion:                 */
/*                                                                            */
/*   a) PROBES (modo runner). Se genera un .c minimo, se compila con           */
/*      -Wall -Wextra -Werror y se reporta OK/KO. Es la unica forma de aislar  */
/*      "falta ft_swap" de "ft_swap tiene otro prototipo", y la unica de       */
/*      comprobar que ft_boolean.h arrastra <unistd.h> para el write() del     */
/*      main del subject. Algunos probes ademas ejecutan el binario y comparan */
/*      su salida.                                                            */
/*                                                                            */
/*   b) CASOS (modo test, -DFT_EX=NN). Igual que en los packs anteriores:      */
/*      este mismo archivo se recompila haciendo #include de lo que entrego el */
/*      alumno y se lanza en un hijo con timeout, para que un segfault en un   */
/*      ejercicio no tumbe los demas. Sirve para lo que si tiene valor en      */
/*      tiempo de ejecucion: macros (EVEN, ABS), la struct de ex03/ex04 y la   */
/*      salida de ft_show_tab.                                                */
/*                                                                            */
/*   ex00 solo tiene probes (una cabecera de prototipos no ejecuta nada), asi  */
/*   que su entrada en g_ex lleva child = 0 y no hay bloque #if FT_EX == 0     */
/*   con casos de verdad.                                                     */
/*                                                                            */
/*   ft_stock_str.h (ex04/ex05) lo pone la moulinette, no el alumno. Aqui se   */
/*   genera una copia canonica en el directorio temporal y se pasa por -I, de  */
/*   modo que el ejercicio compile tanto si el alumno lo entrego como si no.   */
/*   Si el alumno tiene el suyo, el #include "..." del propio fuente lo        */
/*   encuentra antes y gana el suyo, que es lo que interesa probar.            */
/*                                                                            */
/*   Anadir un ejercicio = una entrada en g_ex + (si child) un bloque          */
/*   #if FT_EX == NN + (si hace falta) una funcion probes_exNN.                */
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
/*                             COMUN A LOS DOS MODOS                          */
/* ========================================================================== */

/* Devuelve s entrecomillado y con los no imprimibles escapados, en uno de     */
/* cuatro buffers rotatorios (para poder usarlo varias veces en un printf).    */
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

/* Textos que el subject fija literalmente. */
#define MSG_EVEN "I have an even number of arguments.\n"
#define MSG_ODD "I have an odd number of arguments.\n"

/* ========================================================================== */
/*                                MODO TEST                                   */
/* ========================================================================== */

#ifdef FT_EX

/* Lo que entrego el alumno se incluye tal cual: asi el compilador ve la       */
/* definicion real (o la cabecera real) y un prototipo equivocado sale como    */
/* error de compilacion, no como comportamiento indefinido en ejecucion. Su    */
/* main() llega renombrado por -Dmain=...; lo deshacemos para declarar el      */
/* nuestro.                                                                   */
# ifdef FT_SRC2
#  include FT_SRC2
# endif
# include FT_SRC
# undef main

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

/* -------------------------------- ex00 ----------------------------------- */

# if FT_EX == 0

/* ft.h no tiene nada que ejecutar: se prueba entero desde el runner, con un  */
/* probe de compilacion por prototipo. Este bloque solo existe para que el    */
/* archivo siga compilando si alguien lo monta a mano con -DFT_EX=0.          */
static void	run_cases(void)
{
	printf("  este ejercicio se prueba desde el runner\n");
}

# endif

/* -------------------------------- ex01 ----------------------------------- */

# if FT_EX == 1

static void	t_even(int n, int is_even)
{
	CASE("EVEN(%d) es %s", n, is_even ? "cierto" : "falso");
	if (!res((EVEN(n) != 0) == (is_even != 0)))
		detail("EVEN(%d) vale %d", n, (int)(EVEN(n) != 0));
}

/* EVEN se usa con una expresion, no solo con una variable: sin parentesis    */
/* alrededor del parametro el resultado sale mal.                             */
static void	t_even_expr(void)
{
	CASE("EVEN(3 + 1) es cierto (parentesis alrededor del parametro)");
	if (!res(EVEN(3 + 1) != 0))
		detail("define EVEN(x) usando (x), no x: EVEN(3 + 1) se expande a "
			"3 + 1 %% 2 y da otra cosa");
}

static void	t_msg(const char *got, const char *exp, const char *name)
{
	CASE("%s = %s", name, q(exp));
	if (res(strcmp(got, exp) == 0))
		return ;
	detail("obtenido %s", q(got));
	if (strncmp(got, exp, strlen(exp) - 1) == 0 && strlen(got) == strlen(exp) - 1)
		detail("solo falta el salto de linea final dentro de la propia macro");
}

/* Un CASE por funcion: el macro sale de la funcion cuando toca saltarse el   */
/* caso, asi que dos CASE seguidos descuadrarian la numeracion al reanudar    */
/* despues de un crash.                                                      */
static void	t_distinct(void)
{
	CASE("TRUE y FALSE son valores distintos");
	res(TRUE != FALSE);
}

static void	t_values(void)
{
	CASE("FALSE vale 0 y TRUE vale 1");
	if (!res(FALSE == 0 && TRUE == 1))
		detail("FALSE = %d, TRUE = %d", (int)FALSE, (int)TRUE);
}

static void	t_success(void)
{
	CASE("SUCCESS vale 0 (es lo que devuelve main)");
	if (!res(SUCCESS == 0))
		detail("SUCCESS = %d: el programa saldria con codigo de error",
			(int)SUCCESS);
}

static void	t_bool_type(void)
{
	t_bool	b;

	CASE("t_bool guarda TRUE y FALSE");
	b = TRUE;
	if (b != TRUE)
		return ((void)res(0));
	b = FALSE;
	res(b == FALSE);
}

static void	run_cases(void)
{
	t_distinct();
	t_values();
	t_success();
	t_bool_type();
	t_msg(EVEN_MSG, MSG_EVEN, "EVEN_MSG");
	t_msg(ODD_MSG, MSG_ODD, "ODD_MSG");
	t_even(0, 1);
	t_even(1, 0);
	t_even(2, 1);
	t_even(7, 0);
	t_even(42, 1);
	t_even(-1, 0);
	t_even(-2, 1);
	t_even(-7, 0);
	t_even_expr();
}

# endif

/* -------------------------------- ex02 ----------------------------------- */

# if FT_EX == 2

static void	t_abs(int v, int exp)
{
	int	got;

	CASE("ABS(%d) = %d", v, exp);
	got = ABS(v);
	if (!res(got == exp))
		detail("obtenido %d", got);
}

/* Los dos fallos clasicos de la macro: faltan los parentesis interiores      */
/* (alrededor de Value) o el exterior (alrededor de toda la expansion).       */
static void	t_abs_inner(void)
{
	int	got;

	CASE("ABS(-3 + 1) = 2 (parentesis alrededor de Value)");
	got = ABS(-3 + 1);
	if (!res(got == 2))
		detail("obtenido %d: define ABS(Value) usando (Value), no Value", got);
}

static void	t_abs_outer(void)
{
	int	got;

	CASE("2 * ABS(-3) = 6 (parentesis alrededor de toda la macro)");
	got = 2 * ABS(-3);
	if (!res(got == 6))
		detail("obtenido %d: toda la expansion tiene que ir entre parentesis",
			got);
}

static void	t_abs_var(void)
{
	int	x;
	int	got;

	CASE("con una variable: x = -8, ABS(x) = 8");
	x = -8;
	got = ABS(x);
	if (!res(got == 8))
		detail("obtenido %d", got);
}

static void	t_abs_expr(void)
{
	int	x;
	int	got;

	CASE("con una resta: x = 5, ABS(x - 12) = 7");
	x = 5;
	got = ABS(x - 12);
	if (!res(got == 7))
		detail("obtenido %d", got);
}

static void	run_cases(void)
{
	t_abs(0, 0);
	t_abs(1, 1);
	t_abs(-1, 1);
	t_abs(42, 42);
	t_abs(-42, 42);
	t_abs(2147483647, 2147483647);
	t_abs(-2147483647, 2147483647);
	t_abs_var();
	t_abs_expr();
	t_abs_inner();
	t_abs_outer();
}

# endif

/* -------------------------------- ex03 ----------------------------------- */

# if FT_EX == 3

static void	t_point_fields(void)
{
	t_point	p;

	CASE("t_point tiene x e y y se pueden asignar");
	p.x = 42;
	p.y = 21;
	if (!res(p.x == 42 && p.y == 21))
		detail("x = %d, y = %d", p.x, p.y);
}

static void	t_point_int(void)
{
	t_point	p;
	int		*px;
	int		*py;

	CASE("x e y son int");
	px = &p.x;
	py = &p.y;
	*px = -7;
	*py = 13;
	if (!res(p.x == -7 && p.y == 13))
		detail("x = %d, y = %d", p.x, p.y);
}

/* La struct se pasa por puntero, que es como la usa el main del subject. */
static void	set_point(t_point *point)
{
	point->x = 42;
	point->y = 21;
}

static void	t_point_ptr(void)
{
	t_point	p;

	CASE("se puede rellenar a traves de un t_point *");
	p.x = 0;
	p.y = 0;
	set_point(&p);
	if (!res(p.x == 42 && p.y == 21))
		detail("x = %d, y = %d", p.x, p.y);
}

static void	run_cases(void)
{
	t_point_fields();
	t_point_int();
	t_point_ptr();
}

# endif

/* -------------------------------- ex04 ----------------------------------- */

# if FT_EX == 4

/* Representacion corta del array de entrada, para la etiqueta del caso. */
static const char	*qv(int ac, char **av)
{
	static char	buf[160];
	size_t		k;
	int			i;

	k = (size_t)sprintf(buf, "{");
	i = 0;
	while (i < ac && k < 120)
	{
		k += (size_t)sprintf(buf + k, "%s%s", i ? ", " : "", q(av[i]));
		i++;
	}
	sprintf(buf + k, "}");
	return (buf);
}

/* tab == NULL significa que este caso ya se probo en un intento anterior (o  */
/* que ft_strs_to_tab devolvio NULL, que ya se reporto arriba): se cuenta el  */
/* caso para no descuadrar la numeracion, pero no se toca nada.               */
static void	t_elem(t_stock_str *tab, char **av, int i)
{
	int	len;
	int	ok;

	len = (int)strlen(av[i]);
	CASE("  elemento %d de %s", i, q(av[i]));
	if (!tab)
		return ;
	ok = (tab[i].str && tab[i].copy && tab[i].size == len
			&& strcmp(tab[i].str, av[i]) == 0
			&& strcmp(tab[i].copy, av[i]) == 0
			&& tab[i].copy != tab[i].str);
	if (res(ok))
		return ;
	if (!tab[i].str)
		detail("str es NULL");
	else if (strcmp(tab[i].str, av[i]) != 0)
		detail("str: esperado %s, obtenido %s", q(av[i]), q(tab[i].str));
	if (tab[i].size != len)
		detail("size: esperado %d, obtenido %d", len, tab[i].size);
	if (!tab[i].copy)
		detail("copy es NULL");
	else if (strcmp(tab[i].copy, av[i]) != 0)
		detail("copy: esperado %s, obtenido %s", q(av[i]), q(tab[i].copy));
	else if (tab[i].copy == tab[i].str)
		detail("copy y str apuntan al mismo sitio: copy tiene que ser una "
			"copia aparte, porque el corrector la modifica");
}

/* copy tiene que ser memoria propia: tocarla no puede cambiar str. */
static void	t_independent(t_stock_str *tab, char **av, int ac)
{
	int	i;
	int	ok;

	CASE("  modificar copy no toca str");
	if (!tab)
		return ;
	i = 0;
	while (i < ac)
	{
		if (tab[i].copy && tab[i].copy[0])
			tab[i].copy[0] = 'X';
		i++;
	}
	ok = 1;
	i = 0;
	while (i < ac)
	{
		if (!tab[i].str || strcmp(tab[i].str, av[i]) != 0)
			ok = 0;
		i++;
	}
	if (!res(ok))
		detail("str y copy comparten memoria");
}

static void	t_end(t_stock_str *tab, int ac)
{
	CASE("  el elemento %d cierra la tabla con str = 0", ac);
	if (!tab)
		return ;
	if (!res(tab[ac].str == 0))
		detail("el ultimo elemento tiene que llevar str a 0 para marcar el "
			"final, si no no se sabe donde acaba la tabla");
}

/* Aqui no se usa CASE sino case_begin directamente: CASE sale de la funcion  */
/* al saltarse el caso, y eso dejaria sin contar los casos de los helpers de  */
/* abajo. Todos tienen que llamarse siempre, corran o no.                     */
static void	t_strs(int ac, char **av)
{
	t_stock_str	*tab;
	int			i;

	tab = NULL;
	if (!case_begin("ft_strs_to_tab(%d, %s)", ac, qv(ac, av)))
	{
		tab = ft_strs_to_tab(ac, av);
		if (!res(tab != NULL))
			detail("devuelve NULL sin que haya fallado ningun malloc");
	}
	i = 0;
	while (i < ac)
	{
		t_elem(tab, av, i);
		i++;
	}
	t_end(tab, ac);
	if (ac > 0)
		t_independent(tab, av, ac);
}

static void	run_cases(void)
{
	char	*av1[1];
	char	*av3[3];
	char	*av4[4];

	av1[0] = "hola";
	av3[0] = "uno";
	av3[1] = "dos";
	av3[2] = "tres";
	av4[0] = "";
	av4[1] = "a";
	av4[2] = "cadena con espacios";
	av4[3] = "con\tsalto\ny tab";
	t_strs(0, av1);
	t_strs(1, av1);
	t_strs(3, av3);
	t_strs(4, av4);
	t_strs(2, av3);
}

# endif

/* -------------------------------- ex05 ----------------------------------- */

# if FT_EX == 5

static void	set_elem(t_stock_str *e, char *str, int size, char *copy)
{
	e->str = str;
	e->size = size;
	e->copy = copy;
}

/* Lo que el subject pide por cada elemento: str, size y copy, cada uno       */
/* seguido de un salto de linea, y nada mas.                                  */
static void	t_show(t_stock_str *tab, const char *label)
{
	char	exp[2048];
	int		k;
	int		i;

	CASE("%s", label);
	k = 0;
	i = 0;
	while (tab[i].str)
	{
		k += sprintf(exp + k, "%s\n%d\n%s\n", tab[i].str, tab[i].size,
			tab[i].copy);
		i++;
	}
	exp[k] = '\0';
	cap_start();
	ft_show_tab(tab);
	cap_end();
	cmp_out(exp);
}

static void	run_cases(void)
{
	t_stock_str	tab[5];

	set_elem(&tab[0], 0, 0, 0);
	t_show(tab, "tabla vacia (solo el terminador): no imprime nada");
	set_elem(&tab[0], "hola", 4, "hola");
	set_elem(&tab[1], 0, 0, 0);
	t_show(tab, "un elemento");
	set_elem(&tab[0], "uno", 3, "uno");
	set_elem(&tab[1], "dos", 3, "dos");
	set_elem(&tab[2], "tres", 4, "tres");
	set_elem(&tab[3], 0, 0, 0);
	t_show(tab, "tres elementos, en orden");
	set_elem(&tab[0], "original", 8, "Xriginal");
	set_elem(&tab[1], 0, 0, 0);
	t_show(tab, "copy modificada: se imprime la copy, no str");
	set_elem(&tab[0], "", 0, "");
	set_elem(&tab[1], "a", 1, "a");
	set_elem(&tab[2], 0, 0, 0);
	t_show(tab, "cadenas vacias");
	set_elem(&tab[0], "abc", 99, "abc");
	set_elem(&tab[1], 0, 0, 0);
	t_show(tab, "se imprime el size guardado, no strlen(str)");
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

# include <dirent.h>
# include <signal.h>

# define PATHSZ 4096
# define TIMEOUT 20
/* topes de relanzamiento por ejercicio: si revientan (o se cuelgan) mas casos
   que esto, se corta la tanda en vez de seguir reintentando */
# define MAX_CRASH 12
# define MAX_HANG 1

/* K_HDR: lo que se entrega es una cabecera, no hay .o que sacar.             */
/* K_SRC: lo que se entrega es un .c, como en los packs anteriores.           */
# define K_HDR 0
# define K_SRC 1

typedef struct s_ex
{
	int			id;
	const char	*dir;
	const char	*src;
	const char	*src2;
	int			kind;
	int			child;
	const char	*note;
}	t_ex;

static const t_ex	g_ex[] = {
{0, "ex00", "ft.h", NULL, K_HDR, 0, NULL},
{1, "ex01", "ft_boolean.h", NULL, K_HDR, 1,
	"este ejercicio se corrige con norminette -R CheckDefine"},
{2, "ex02", "ft_abs.h", NULL, K_HDR, 1,
	"este ejercicio se corrige con norminette -R CheckDefine"},
{3, "ex03", "ft_point.h", NULL, K_HDR, 1, NULL},
{4, "ex04", "ft_strs_to_tab.c", NULL, K_SRC, 1,
	"ft_stock_str.h lo pone la moulinette; aqui se usa el tuyo si lo tienes"},
{5, "ex05", "ft_show_tab.c", NULL, K_SRC, 1,
	"ft_stock_str.h lo pone la moulinette; aqui se usa el tuyo si lo tienes"},
};
# define N_EX (sizeof(g_ex) / sizeof(g_ex[0]))

static char			g_tmp[64];
static char			g_exdir[PATHSZ];
static char			g_src[PATHSZ];
static char			g_lastlog[PATHSZ];
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

/* En C08 un fallo se propaga: una cabecera vacia dispara treinta errores del  */
/* compilador y el log tapa el resto del informe. Se cortan las primeras       */
/* MAX_LOG lineas, que son las que dicen algo.                                 */
# define MAX_LOG 12

static void	dump_log(const char *path)
{
	char	*log;
	char	*p;
	int		n;

	log = slurp(path);
	if (!log || !*log)
		return (free(log));
	n = 0;
	p = log;
	while (*p && n < MAX_LOG)
		if (*p++ == '\n')
			n++;
	if (*p)
	{
		*p = '\0';
		printf("%s%s         (log recortado)%s\n", C_D, log, C_0);
	}
	else
		printf("%s%s%s", C_D, log, C_0);
	free(log);
}

/* ------------------------------ reporte ---------------------------------- */
/* Mismo formato que los casos del hijo, para que la salida se lea igual.     */

static void	pr_ok(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

static void	pr_ok(const char *fmt, ...)
{
	va_list	ap;

	printf("  %s[OK]%s   ", C_OK, C_0);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
}

static void	pr_ko(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

static void	pr_ko(const char *fmt, ...)
{
	va_list	ap;

	printf("  %s[KO]%s   ", C_KO, C_0);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
}

static void	pr_detail(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));

static void	pr_detail(const char *fmt, ...)
{
	va_list	ap;

	printf("         %s", C_D);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("%s\n", C_0);
}

/* ------------------------------- probes ---------------------------------- */
/* Un probe es un .c generado al vuelo que se compila con los mismos flags    */
/* que la moulinette. Es lo unico que distingue "no esta declarado" de "esta  */
/* declarado con otro tipo", que es justo lo que se corrige en C08.           */

/* link = 0 se queda en -fsyntax-only. Importa: un probe que solo quiere saber */
/* si algo esta declarado y con que tipo no puede enlazar, porque tomar la     */
/* direccion de una funcion declarada pero sin implementar daria "undefined    */
/* reference" y se confundiria un prototipo correcto con uno que falta.        */
static int	build_src(const char *tag, const char *code, char *bin, size_t n,
		int link)
{
	char	src[PATHSZ];
	char	cmd[PATHSZ * 4];
	FILE	*f;

	xsnprintf(src, sizeof(src), "%s/%s.c", g_tmp, tag);
	xsnprintf(g_lastlog, sizeof(g_lastlog), "%s/%s.log", g_tmp, tag);
	xsnprintf(bin, n, "%s/%s.bin", g_tmp, tag);
	f = fopen(src, "w");
	if (!f)
		return (1);
	fputs(code, f);
	fclose(f);
	if (link)
		xsnprintf(cmd, sizeof(cmd),
			"cc -Wall -Wextra -Werror -I '%s' -I '%s' -o '%s' '%s' > '%s' 2>&1",
			g_exdir, g_tmp, bin, src, g_lastlog);
	else
		xsnprintf(cmd, sizeof(cmd),
			"cc -Wall -Wextra -Werror -I '%s' -I '%s' -fsyntax-only '%s' "
			"> '%s' 2>&1", g_exdir, g_tmp, src, g_lastlog);
	if (system(cmd) != 0)
		return (1);
	return (0);
}

/* Compila un main() con el cuerpo dado, despues de incluir lo que entrego el */
/* alumno. Devuelve 1 si no compila (o sea, tests KO que sumar).              */
static int	probe(const char *label, const char *body)
{
	char	code[PATHSZ * 2];
	char	bin[PATHSZ];

	xsnprintf(code, sizeof(code),
		"#include \"%s\"\n"
		"\n"
		"int\tmain(void)\n"
		"{\n"
		"%s\n"
		"\treturn (0);\n"
		"}\n", g_src, body);
	if (build_src("probe", code, bin, sizeof(bin), 0) == 0)
	{
		pr_ok("%s", label);
		return (0);
	}
	pr_ko("%s", label);
	dump_log(g_lastlog);
	return (1);
}

/* Igual, pero lo que no compile es solo un aviso: para lo que el subject no  */
/* pide de forma explicita pero se suele esperar en la defensa.               */
static void	probe_opt(const char *label, const char *body)
{
	char	code[PATHSZ * 2];
	char	bin[PATHSZ];

	xsnprintf(code, sizeof(code),
		"#include \"%s\"\n"
		"\n"
		"int\tmain(void)\n"
		"{\n"
		"%s\n"
		"\treturn (0);\n"
		"}\n", g_src, body);
	if (build_src("probeopt", code, bin, sizeof(bin), 0) == 0)
		return (pr_ok("%s", label));
	printf("  %s[AVISO]%s %s\n", C_SK, C_0, label);
}

/* Ejecuta un binario ya compilado con unos argumentos y compara su salida.   */
static int	run_out(const char *bin, const char *args, const char *exp,
		const char *label)
{
	char	out[PATHSZ];
	char	cmd[PATHSZ * 2];
	char	*got;
	int		pass;

	xsnprintf(out, sizeof(out), "%s/probe.out", g_tmp);
	xsnprintf(cmd, sizeof(cmd), "'%s' %s > '%s' 2>&1", bin, args, out);
	if (system(cmd) == -1)
		return (pr_ko("%s", label), 1);
	got = slurp(out);
	pass = (got && strcmp(got, exp) == 0);
	if (pass)
		pr_ok("%s", label);
	else
	{
		pr_ko("%s", label);
		pr_detail("esperado %s", q(exp));
		pr_detail("obtenido %s", q(got ? got : ""));
	}
	free(got);
	return (!pass);
}

/* Incluir la cabecera dos veces no tiene por que compilar (la moulinette la  */
/* incluye una sola vez), pero en la defensa se pregunta, asi que se avisa.   */
static void	check_guard(const t_ex *ex)
{
	char	code[PATHSZ * 3];
	char	bin[PATHSZ];

	xsnprintf(code, sizeof(code),
		"#include \"%s\"\n#include \"%s\"\n"
		"\nint\tmain(void)\n{\n\treturn (0);\n}\n", g_src, g_src);
	if (build_src("guard", code, bin, sizeof(bin), 0) == 0)
		return ;
	printf("  %s[AVISO]%s incluir %s dos veces no compila: le falta el include "
		"guard (#ifndef ... # define ... #endif)\n", C_SK, C_0, ex->src);
}

/* ------------------------------ probes ex00 ------------------------------- */
/* Tomar la direccion de la funcion con el tipo exacto del subject: si no     */
/* esta declarada da "undeclared", y si el prototipo no coincide da           */
/* "incompatible pointer types". Uno por prototipo, para saber cual falla.    */

typedef struct s_proto
{
	const char	*label;
	const char	*body;
}	t_proto;

static const t_proto	g_protos[] = {
{"void ft_putchar(char c);", "\tvoid\t(*p)(char) = ft_putchar;\n\n\t(void)p;"},
{"void ft_swap(int *a, int *b);",
	"\tvoid\t(*p)(int *, int *) = ft_swap;\n\n\t(void)p;"},
{"void ft_putstr(char *str);", "\tvoid\t(*p)(char *) = ft_putstr;\n\n\t(void)p;"},
{"int ft_strlen(char *str);", "\tint\t(*p)(char *) = ft_strlen;\n\n\t(void)p;"},
{"int ft_strcmp(char *s1, char *s2);",
	"\tint\t(*p)(char *, char *) = ft_strcmp;\n\n\t(void)p;"},
};

static int	probes_ex00(void)
{
	size_t	i;
	int		ko;

	ko = 0;
	i = 0;
	while (i < sizeof(g_protos) / sizeof(g_protos[0]))
	{
		ko += probe(g_protos[i].label, g_protos[i].body);
		i++;
	}
	return (ko);
}

/* ------------------------------ probes ex01 ------------------------------- */
/* El main del subject, copiado literal. Es la prueba de fuego del ejercicio: */
/* usa write() sin incluir <unistd.h>, asi que tiene que incluirlo la propia  */
/* ft_boolean.h.                                                              */

static const char	*g_main01 =
	"#include \"%s\"\n"
	"\n"
	"void\tft_putstr(char *str)\n"
	"{\n"
	"\twhile (*str)\n"
	"\t\twrite(1, str++, 1);\n"
	"}\n"
	"\n"
	"t_bool\tft_is_even(int nbr)\n"
	"{\n"
	"\treturn ((EVEN(nbr)) ? TRUE : FALSE);\n"
	"}\n"
	"\n"
	"int\tmain(int argc, char **argv)\n"
	"{\n"
	"\t(void)argv;\n"
	"\tif (ft_is_even(argc - 1) == TRUE)\n"
	"\t\tft_putstr(EVEN_MSG);\n"
	"\telse\n"
	"\t\tft_putstr(ODD_MSG);\n"
	"\treturn (SUCCESS);\n"
	"}\n";

static void	hint_write(void)
{
	char	*txt;

	txt = slurp(g_lastlog);
	if (txt && strstr(txt, "write"))
		pr_detail("el main del subject llama a write() sin incluir "
			"<unistd.h>: tiene que incluirlo ft_boolean.h");
	free(txt);
}

static int	probes_ex01(void)
{
	char	code[PATHSZ * 2];
	char	bin[PATHSZ];
	int		ko;

	xsnprintf(code, sizeof(code), g_main01, g_src);
	if (build_src("main01", code, bin, sizeof(bin), 1) != 0)
	{
		pr_ko("el main del subject compila con -Wall -Wextra -Werror");
		dump_log(g_lastlog);
		hint_write();
		return (1);
	}
	pr_ok("el main del subject compila con -Wall -Wextra -Werror");
	ko = run_out(bin, "", MSG_EVEN, "sin argumentos (argc - 1 = 0) -> even");
	ko += run_out(bin, "a", MSG_ODD, "1 argumento -> odd");
	ko += run_out(bin, "a b", MSG_EVEN, "2 argumentos -> even");
	ko += run_out(bin, "a b c", MSG_ODD, "3 argumentos -> odd");
	probe_opt("ODD tambien esta definida (el subject no la pide, pero se "
		"pregunta en la defensa)", "\tint\tv = ODD(3);\n\n\t(void)v;");
	return (ko);
}

/* ------------------------------ probes ex03 ------------------------------- */

static const char	*g_main03 =
	"#include \"%s\"\n"
	"\n"
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
	"}\n";

static int	probes_ex03(void)
{
	char	code[PATHSZ * 2];
	char	bin[PATHSZ];

	xsnprintf(code, sizeof(code), g_main03, g_src);
	if (build_src("main03", code, bin, sizeof(bin), 1) == 0)
	{
		pr_ok("el main del subject compila con -Wall -Wextra -Werror");
		return (0);
	}
	pr_ko("el main del subject compila con -Wall -Wextra -Werror");
	dump_log(g_lastlog);
	return (1);
}

static int	run_probes(const t_ex *ex)
{
	if (ex->kind == K_HDR)
		check_guard(ex);
	if (ex->id == 0)
		return (probes_ex00());
	if (ex->id == 1)
		return (probes_ex01());
	if (ex->id == 3)
		return (probes_ex03());
	return (0);
}

/* ------------------------------- entorno ---------------------------------- */

/* Ruta absoluta de p (hace falta para el #include de lo que entrego el       */
/* alumno, que se resuelve respecto al directorio de test.c, no al cwd).      */
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

/* ft_stock_str.h de ex04/ex05 lo pone la moulinette. Se deja una copia en el */
/* temporal y se pasa por -I: si el alumno tiene el suyo en el directorio del */
/* ejercicio, el #include "..." de su propio fuente lo encuentra antes.       */
static void	make_stock_str(void)
{
	char	path[PATHSZ];
	FILE	*f;

	xsnprintf(path, sizeof(path), "%s/ft_stock_str.h", g_tmp);
	f = fopen(path, "w");
	if (!f)
		return ;
	fprintf(f, "#ifndef FT_STOCK_STR_H\n# define FT_STOCK_STR_H\n\n"
		"typedef struct s_stock_str\n{\n\tint\t\tsize;\n"
		"\tchar\t*str;\n\tchar\t*copy;\n}\tt_stock_str;\n\n#endif\n");
	fclose(f);
}

static int	is_expected(const t_ex *ex, const char *name)
{
	if (strcmp(name, ex->src) == 0)
		return (1);
	if (ex->src2 && strcmp(name, ex->src2) == 0)
		return (1);
	if (ex->kind == K_SRC && strcmp(name, "ft_stock_str.h") == 0)
		return (1);
	return (1 - 1);
}

/* Avisa de los .c y .h del directorio que el subject no pide. Cuando ademas  */
/* falta el archivo principal casi siempre es que esta mal el nombre.         */
static void	scan_dir(const char *repo, const t_ex *ex)
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
		if (len > 2 && (!strcmp(e->d_name + len - 2, ".c")
				|| !strcmp(e->d_name + len - 2, ".h"))
			&& !is_expected(ex, e->d_name))
			printf("  %s[AVISO]%s sobra %s: el subject solo pide %s\n",
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

/* ------------------------------ ejecucion --------------------------------- */

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

/* Paso previo comun: que lo entregado compile suelto con los flags de la     */
/* moulinette. Para un .c es el -c de siempre; para una cabecera hay que      */
/* envolverla en un .c, porque cc con un .h suelto genera un precompilado.    */
static int	compile_alone(const t_ex *ex)
{
	char	code[PATHSZ * 2];
	char	obj[PATHSZ];
	char	cmd[PATHSZ * 4];

	if (ex->kind == K_HDR)
	{
		xsnprintf(code, sizeof(code),
			"#include \"%s\"\n\nint\tmain(void)\n{\n\treturn (0);\n}\n", g_src);
		if (build_src("alone", code, obj, sizeof(obj), 0) == 0)
			return (1);
		printf("  %s[COMPILA KO]%s %s con -Wall -Wextra -Werror\n",
			C_KO, C_0, ex->src);
		return (dump_log(g_lastlog), 0);
	}
	xsnprintf(g_lastlog, sizeof(g_lastlog), "%s/%s.log", g_tmp, ex->dir);
	xsnprintf(obj, sizeof(obj), "%s/%s.o", g_tmp, ex->dir);
	xsnprintf(cmd, sizeof(cmd), "cc -Wall -Wextra -Werror -I '%s' -I '%s' "
		"-c -o '%s' '%s' > '%s' 2>&1", g_exdir, g_tmp, obj, g_src, g_lastlog);
	if (system(cmd) != 0)
	{
		printf("  %s[COMPILA KO]%s con -Wall -Wextra -Werror\n", C_KO, C_0);
		return (dump_log(g_lastlog), 0);
	}
	return (warn_if_main(obj), 1);
}

/* Monta el hijo: este mismo test.c recompilado con -DFT_EX=NN, incluyendo lo */
/* que entrego el alumno. 0 = no se pudo montar.                              */
static int	build_child(const t_ex *ex, const char *self, char *bin, size_t n)
{
	char	cmd[PATHSZ * 5];

	xsnprintf(g_lastlog, sizeof(g_lastlog), "%s/%s.test.log", g_tmp, ex->dir);
	xsnprintf(bin, n, "%s/%s.bin", g_tmp, ex->dir);
	/* -Wno-return-type: al renombrar main() deja de aplicarsele el trato
	   especial del estandar y "cae" sin return; ya se comprobo intacto arriba */
	xsnprintf(cmd, sizeof(cmd),
		"cc -Wall -Wextra -Werror -Wno-return-type -DFT_EX=%d -DFT_SRC='\"%s\"' "
		"-I '%s' -I '%s' -Dmain=ft_test_unused_main -o '%s' '%s' > '%s' 2>&1",
		ex->id, g_src, g_exdir, g_tmp, bin, self, g_lastlog);
	if (system(cmd) == 0)
		return (1);
	printf("  %s[TEST KO]%s no se pudo montar el test (falta algo de lo que "
		"pide el subject, o tiene otro tipo)\n", C_KO, C_0);
	return (dump_log(g_lastlog), 0);
}

/* Devuelve el numero de tests KO, -1 si el ejercicio no se pudo probar,      */
/* -2 si lo que pide el subject no existe (SKIP).                            */
static int	do_ex(const char *repo, const char *self, const t_ex *ex)
{
	char	path[PATHSZ];
	char	bin[PATHSZ];
	char	casef[PATHSZ];
	int		ko;
	int		r;

	printf("%s── %s/%s%s\n", C_B, ex->dir, ex->src, C_0);
	xsnprintf(path, sizeof(path), "%s/%s", repo, ex->dir);
	if (!abs_path(path, g_exdir, sizeof(g_exdir)))
		return (-1);
	xsnprintf(path, sizeof(path), "%s/%s", g_exdir, ex->src);
	xsnprintf(g_src, sizeof(g_src), "%s", path);
	if (access(g_src, R_OK) != 0)
	{
		printf("  %s[SKIP]%s no encontrado (%s/%s/%s)\n", C_SK, C_0, repo,
			ex->dir, ex->src);
		scan_dir(repo, ex);
		return (-2);
	}
	scan_dir(repo, ex);
	/* la nota va aqui y no al final: zentest -c se lleva las lineas sangradas
	   que van detras de un [KO] como detalle de ese caso */
	if (ex->note)
		printf("  %s%s%s\n", C_D, ex->note, C_0);
	if (!compile_alone(ex))
		return (-1);
	ko = run_probes(ex);
	if (!ex->child)
		return (ko);
	if (!build_child(ex, self, bin, sizeof(bin)))
		return (ko > 0 ? ko : -1);
	xsnprintf(casef, sizeof(casef), "%s/%s.case", g_tmp, ex->dir);
	r = run_bin(bin, casef);
	if (r < 0)
		return (ko > 0 ? ko : -1);
	return (ko + r);
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

static void	summary(int ko_ex, int skipped, int ko_tests)
{
	printf("%s%zu ejercicios%s  %s%d con fallos%s  %s%d sin entregar%s",
		C_B, N_EX, C_0, C_KO, ko_ex, C_0, C_SK, skipped, C_0);
	if (ko_tests)
		printf("  %s(%d tests KO)%s", C_KO, ko_tests, C_0);
	printf("\n");
}

int	main(int argc, char **argv)
{
	const char	*repo;
	char		self[PATHSZ];
	int			n[3];
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
	strcpy(g_tmp, "/tmp/c08test.XXXXXX");
	if (!mkdtemp(g_tmp))
		return (perror("mkdtemp"), 1);
	make_stub();
	make_stock_str();
	printf("repo: %s\ntest: %s\n\n", repo, self);
	n[0] = 0;
	n[1] = 0;
	n[2] = 0;
	i = 0;
	while (i < N_EX)
	{
		r = do_ex(repo, self, &g_ex[i]);
		if (r == -2)
			n[2]++;
		else if (r != 0)
			n[1]++;
		if (r > 0)
			n[0] += r;
		i++;
		printf("\n");
	}
	summary(n[1], n[2], n[0]);
	cleanup();
	return (n[1] != 0);
}

#endif
