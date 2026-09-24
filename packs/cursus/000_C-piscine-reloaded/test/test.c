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
	EX_SHELL = 1
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
