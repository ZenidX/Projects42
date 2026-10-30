/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para get_next_line                              */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*              (o desde test/:  make)                                        */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*              (o desde test/:  make test)                                   */
/*                                                                            */
/*   Este archivo hace dos papeles:                                           */
/*                                                                            */
/*   a) RUNNER (compilacion normal). Para la parte obligatoria y la bonus:    */
/*      - que esten los tres ficheros y pasen norminette;                     */
/*      - que compile sin -D BUFFER_SIZE (el .h tiene que poner uno por       */
/*        defecto) y con -D BUFFER_SIZE=1, 42 y 10000000;                     */
/*      - con nm: que defina get_next_line, que no haya variables globales,   */
/*        que no llame a nada fuera de read, malloc y free, que no tape una   */
/*        funcion de la libc y, en el bonus, que haya una sola static;        */
/*      - por cada BUFFER_SIZE se recompila a si mismo con -DFT_CASES junto   */
/*        a los .o del alumno y lanza los casos.                              */
/*                                                                            */
/*   b) CASOS (-DFT_CASES). Cada caso corre en un hijo con timeout, asi que   */
/*      el estado static de get_next_line empieza de cero en cada uno y un    */
/*      segfault o un bucle infinito no tumban la tanda. malloc/free estan    */
/*      vigilados como en el runner de Libft: escribir pasado el final de un  */
/*      bloque es KO, una linea sin '\0' tambien, y un caso que pasa no puede */
/*      dejar memoria viva. Todos los casos leen hasta el NULL final, que es  */
/*      cuando get_next_line ya no tiene por que guardar nada. Ademas se      */
/*      repite una lectura haciendo fallar el malloc numero k: tiene que      */
/*      acabar en NULL sin reventar y sin leaks.                              */
/*                                                                            */
/*   Anadir un fichero de prueba = una linea en build_files(). Anadir un      */
/*   caso = una funcion c_* y su run() en suite_mand() o suite_bonus().       */
/*                                                                            */
/* ************************************************************************** */

#define _DEFAULT_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define CFLAGS	"-Wall -Wextra -Werror"

static const char	*C_OK = "";
static const char	*C_KO = "";
static const char	*C_SK = "";
static const char	*C_B = "";
static const char	*C_0 = "";

static void	init_colors(void)
{
	if (!isatty(STDOUT_FILENO))
		return ;
	C_OK = "\033[32m";
	C_KO = "\033[31m";
	C_SK = "\033[33m";
	C_B = "\033[1m";
	C_0 = "\033[0m";
}

#ifndef FT_CASES

/* ================================ RUNNER ================================= */

typedef struct s_variant
{
	const char	*name;		/* cabecera de la seccion de comprobaciones */
	const char	*tag;		/* prefijo de las secciones y de los temporales */
	const char	*src[2];
	const char	*hdr;
	int			one_static;
}	t_variant;

static const t_variant	g_var[] = {
	{"get_next_line", "", {"get_next_line.c", "get_next_line_utils.c"},
		"get_next_line.h", 0},
	{"bonus", "bonus/", {"get_next_line_bonus.c",
		"get_next_line_utils_bonus.c"}, "get_next_line_bonus.h", 1},
};
#define N_VAR	(sizeof(g_var) / sizeof(*g_var))

/* El 10000000 es el que tumba a quien pone el buffer en la pila (8 MB).   */
static const char	*g_bs[] = {"1", "42", "10000000"};
#define N_BS	(sizeof(g_bs) / sizeof(*g_bs))

static int		g_has_norm;
static char		*g_libc;
static char		g_tmp[] = "/tmp/gnl_runner.XXXXXX";

/* Ejecuta cmd y devuelve su salida (stdout+stderr) en memoria nueva.        */
static char	*capture(const char *cmd, int *status)
{
	FILE	*p;
	char	*out;
	size_t	len;
	size_t	cap;
	size_t	r;

	cap = 4096;
	len = 0;
	out = malloc(cap);
	p = popen(cmd, "r");
	if (!out || !p)
	{
		*status = -1;
		if (p)
			pclose(p);
		if (out)
			out[0] = '\0';
		return (out);
	}
	while ((r = fread(out + len, 1, cap - len - 1, p)) > 0)
	{
		len += r;
		if (cap - len < 512)
		{
			cap *= 2;
			out = realloc(out, cap);
		}
	}
	out[len] = '\0';
	*status = pclose(p);
	return (out);
}

/* Ejecuta cmd en sh y devuelve su estado; la salida queda en *out.         */
static int	sh(char **out, const char *fmt, ...)
{
	char	cmd[16384];
	va_list	ap;
	int		st;

	va_start(ap, fmt);
	vsnprintf(cmd, sizeof(cmd), fmt, ap);
	va_end(ap);
	free(*out);
	*out = capture(cmd, &st);
	return (st);
}

static void	print_indented(const char *log)
{
	const char	*p;
	const char	*nl;

	p = log;
	while (p && *p)
	{
		nl = strchr(p, '\n');
		if (!nl)
			nl = p + strlen(p);
		printf("    %.*s\n", (int)(nl - p), p);
		p = *nl ? nl + 1 : nl;
	}
}

/* [OK]/[KO] con el texto y, si falla, el detalle sangrado debajo.         */
static int	line(int ok, const char *log, const char *fmt, ...)
{
	va_list	ap;

	printf("  %s%s%s   ", ok ? C_OK : C_KO, ok ? "[OK]" : "[KO]", C_0);
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
	printf("\n");
	if (!ok && log && *log)
		print_indented(log);
	return (!ok);
}

static void	add_name(char **list, const char *name)
{
	size_t	len;

	len = *list ? strlen(*list) : 0;
	*list = realloc(*list, len + strlen(name) + 3);
	if (len == 0)
		(*list)[0] = '\0';
	strcat(strcat(*list, len ? ", " : ""), name);
}

/* Quita las secuencias de color "\033[...m" que mete norminette.          */
static void	strip_ansi(char *s)
{
	char	*e;

	while ((s = strchr(s, '\033')))
	{
		e = strchr(s, 'm');
		if (!e)
		{
			*s = '\0';
			return ;
		}
		memmove(s, e + 1, strlen(e + 1) + 1);
	}
}

/* Errores de norminette de un archivo, una linea por error y sin colores. */
/* NULL si pasa.                                                             */
static char	*norm_errors(const char *path)
{
	char	cmd[4200];
	char	*out;
	char	*res;
	char	*ln;
	char	*w;
	size_t	len;
	int		st;

	snprintf(cmd, sizeof(cmd), "norminette '%s' 2>&1", path);
	out = capture(cmd, &st);
	res = NULL;
	ln = strtok(out, "\n");
	while (ln)
	{
		if (!strncmp(ln, "Error:", 6) || !strncmp(ln, "Error ", 6))
		{
			strip_ansi(ln);
			w = ln + 6;
			while (*w == ' ')
				w++;
			for (char *t = w; *t; t++)
				if (*t == '\t')
					*t = ' ';
			len = res ? strlen(res) : 0;
			res = realloc(res, len + strlen(w) + 2);
			res[len] = '\0';
			strcat(strcat(res, w), "\n");
		}
		ln = strtok(NULL, "\n");
	}
	free(out);
	return (res);
}

/* Nombres que exporta la libc, rodeados por ' ' para buscar " x " con      */
/* strstr. Un get_next_line_utils.c que defina strlen o strjoin a secas      */
/* tapa a la de la libc en todo el programa que lo enlace.                  */
static void	load_libc(void)
{
	char	*out;

	out = NULL;
	sh(&out, "lib=$(ldd /bin/true 2>/dev/null | awk '/libc\\.so/ {print $3}');"
		" nm -D --defined-only \"${lib:-/lib/x86_64-linux-gnu/libc.so.6}\""
		" 2>/dev/null | awk 'NF == 3 { sub(/@.*/, \"\", $3); print $3 }'"
		" | tr '\\n' ' '");
	g_libc = malloc(strlen(out) + 3);
	sprintf(g_libc, " %s ", out);
	free(out);
}

static int	exists(const char *repo, const char *f)
{
	char	path[4096];

	snprintf(path, sizeof(path), "%s/%s", repo, f);
	return (access(path, F_OK) == 0);
}

/* Directorio de los temporales de una variante y un BUFFER_SIZE.          */
static void	work_dir(char *buf, size_t sz, const t_variant *v, const char *bs)
{
	snprintf(buf, sz, "%s/%s%s", g_tmp, *v->tag ? "bonus_" : "mand_", bs);
}

/* Compila los .c que existan, cada uno a su .o. bs NULL = sin -D. -fPIE   */
/* porque el binario de casos es PIE (por los weak) y clang no lo pone solo. */
/* Devuelve 0 si todos compilan; *log se queda con la salida de cc.          */
static int	compile_set(const char *repo, const t_variant *v, const char *bs,
	char **log)
{
	char	dir[4096];
	char	def[64];
	int		bad;

	work_dir(dir, sizeof(dir), v, bs ? bs : "def");
	def[0] = '\0';
	if (bs)
		snprintf(def, sizeof(def), " -D BUFFER_SIZE=%s", bs);
	bad = 0;
	free(*log);
	*log = calloc(1, 1);
	for (int k = 0; k < 2; k++)
	{
		char	*out = NULL;
		size_t	len;

		if (!exists(repo, v->src[k]))
			continue ;
		if (sh(&out, "mkdir -p '%s' && cc " CFLAGS " -fPIE%s -I'%s' -c '%s/%s'"
				" -o '%s/%d.o' 2>&1", dir, def, repo, repo, v->src[k], dir,
				k) != 0)
			bad = 1;
		len = strlen(*log);
		*log = realloc(*log, len + strlen(out) + 1);
		strcpy(*log + len, out);
		free(out);
	}
	return (bad);
}

/* ------------------------------ nm ---------------------------------------- */

typedef struct s_syms
{
	char	*def;		/* " a b c ": funciones definidas (T/W) */
	char	*und;		/* " a b c ": llamadas a fuera (U) */
	char	*glob;		/* variables globales */
	char	*stat;		/* variables static */
	int		nstat;
}	t_syms;

static void	scan_obj(const char *obj, t_syms *s)
{
	char	*out;
	char	*ln;
	char	name[512];
	char	type;

	out = NULL;
	sh(&out, "nm '%s' 2>/dev/null", obj);
	ln = strtok(out, "\n");
	while (ln)
	{
		char	t1[512];
		char	t2[512];
		int		n;

		/* "addr T nombre", o "U nombre" sin direccion en los indefinidos.  */
		n = sscanf(ln, "%511s %511s %511s", t1, t2, name);
		if (n == 2)
			strcpy(name, t2);
		type = n == 3 ? t2[0] : t1[0];
		if ((n == 2 || n == 3) && (n == 2 ? t1[1] : t2[1]) == '\0')
		{
			char	**dst = NULL;

			if (type == 'T' || type == 'W')
				dst = &s->def;
			else if (type == 'U')
				dst = &s->und;
			else if (strchr("BCDGRS", type))
				add_name(&s->glob, name);
			else if (strchr("bds", type) && name[0] != '.')
			{
				add_name(&s->stat, name);
				s->nstat++;
			}
			if (dst)
			{
				size_t	len = *dst ? strlen(*dst) : 0;

				*dst = realloc(*dst, len + strlen(name) + 3);
				if (len == 0)
					strcpy(*dst, " ");
				strcat(strcat(*dst, name), " ");
			}
		}
		ln = strtok(NULL, "\n");
	}
	free(out);
}

static int	has(const char *list, const char *name)
{
	char	key[600];

	if (!list)
		return (0);
	snprintf(key, sizeof(key), " %s ", name);
	return (strstr(list, key) != NULL);
}

/* clang mete un memset/memcpy al inicializar un array con = {0}: si la      */
/* palabra no sale en el fuente, no la ha escrito el alumno.                 */
static int	in_sources(const char *repo, const t_variant *v, const char *fn)
{
	char	*out;
	int		st;

	out = NULL;
	st = sh(&out, "cd '%s' && cat %s %s %s 2>/dev/null | grep -qw '%s'", repo,
			v->src[0], v->src[1], v->hdr, fn);
	free(out);
	return (st == 0);
}

static int	check_syms(const char *repo, const t_variant *v)
{
	static const char	*ok_ext = " read malloc free __stack_chk_fail"
		" _GLOBAL_OFFSET_TABLE_ ";
	char				dir[4096];
	char				obj[4200];
	char				*forbid;
	char				*clash;
	char				*w;
	t_syms				s;
	int					bad;

	memset(&s, 0, sizeof(s));
	work_dir(dir, sizeof(dir), v, "42");
	for (int k = 0; k < 2; k++)
	{
		snprintf(obj, sizeof(obj), "%s/%d.o", dir, k);
		if (access(obj, F_OK) == 0)
			scan_obj(obj, &s);
	}
	bad = line(has(s.def, "get_next_line"), NULL, "define get_next_line");
	if (!has(s.def, "get_next_line"))
		printf("         (revisa el nombre y que no sea static)\n");
	bad |= line(!s.glob, NULL, "sin variables globales");
	if (s.glob)
		printf("         globales: %s (el subject las prohibe)\n", s.glob);
	if (v->one_static)
	{
		bad |= line(s.nstat <= 1, NULL, "una sola variable static");
		if (s.nstat > 1)
			printf("         hay %d: %s\n", s.nstat, s.stat);
	}
	forbid = NULL;
	for (w = s.und ? strtok(s.und, " ") : NULL; w; w = strtok(NULL, " "))
	{
		if (has(ok_ext, w) || has(s.def, w))
			continue ;
		if ((!strcmp(w, "memset") || !strcmp(w, "memcpy")
				|| !strcmp(w, "memmove")) && !in_sources(repo, v, w))
			continue ;
		add_name(&forbid, w);
	}
	bad |= line(!forbid, NULL, "solo llama a read, malloc y free");
	if (forbid)
		printf("         usa %s\n", forbid);
	clash = NULL;
	for (w = s.def ? strtok(s.def, " ") : NULL; w; w = strtok(NULL, " "))
		if (has(g_libc, w))
			add_name(&clash, w);
	bad |= line(!clash, NULL, "no redefine funciones de la libc");
	if (clash)
		printf("         define %s, que tapa a la de la libc: ponle otro"
			" nombre (ft_...)\n", clash);
	free(forbid);
	free(clash);
	free(s.def);
	free(s.und);
	free(s.glob);
	free(s.stat);
	return (bad);
}

/* --------------------------- secciones ----------------------------------- */

/* El .h tiene que dar un BUFFER_SIZE cuando no llega por -D: se compila un  */
/* programa que lo imprime, y de paso se ve cuanto vale.                    */
static int	check_default(const char *repo, const t_variant *v)
{
	char	probe[4200];
	char	*out;
	int		bad;

	out = NULL;
	snprintf(probe, sizeof(probe), "%s/probe_%s", g_tmp,
		*v->tag ? "bonus" : "mand");
	sh(&out, "printf '#include \"%s\"\\n#include <stdio.h>\\nint main(void)"
		" { printf(\"%%%%ld\", (long)(BUFFER_SIZE)); return (0); }\\n' > '%s.c'",
		v->hdr, probe);
	if (sh(&out, "cc " CFLAGS " -I'%s' '%s.c' -o '%s' 2>&1", repo, probe,
			probe) != 0)
	{
		bad = line(0, out, "sin -D BUFFER_SIZE: el .h tiene que poner un valor"
				" por defecto (#ifndef BUFFER_SIZE)");
		free(out);
		return (bad);
	}
	sh(&out, "'%s'", probe);
	bad = line(atol(out) > 0, NULL, "sin -D BUFFER_SIZE vale %s", out);
	if (!compile_set(repo, v, NULL, &out))
		bad |= line(1, NULL, "los .c compilan sin -D BUFFER_SIZE");
	else
		bad |= line(0, out, "los .c no compilan sin -D BUFFER_SIZE");
	free(out);
	return (bad);
}

/* Ficheros, norma, BUFFER_SIZE por defecto y simbolos. 1 si algo falla.  */
static int	check_static(const char *repo, const t_variant *v)
{
	const char	*files[3] = {v->src[0], v->src[1], v->hdr};
	char		path[4096];
	char		*miss;
	char		*log;
	int			bad;

	miss = NULL;
	for (int k = 0; k < 3; k++)
		if (!exists(repo, files[k]))
			add_name(&miss, files[k]);
	bad = line(!miss, NULL, "estan %s, %s y %s", files[0], files[1], files[2]);
	if (miss)
		printf("         falta %s\n", miss);
	free(miss);
	for (int k = 0; k < 3 && g_has_norm; k++)
	{
		if (!exists(repo, files[k]))
			continue ;
		snprintf(path, sizeof(path), "%s/%s", repo, files[k]);
		log = norm_errors(path);
		bad |= line(!log, log, "norminette %s", files[k]);
		free(log);
	}
	if (!exists(repo, v->hdr))
		return (1);
	bad |= check_default(repo, v);
	log = NULL;
	if (compile_set(repo, v, "42", &log) == 0)
		bad |= check_syms(repo, v);
	else
		bad |= line(0, NULL, "no compila con -D BUFFER_SIZE=42: sin .o no se"
				" pueden mirar los simbolos");
	free(log);
	return (bad);
}

static void	self_path(char *buf, size_t sz, const char *argv0)
{
	const char	*slash;

	slash = strrchr(argv0, '/');
	if (!slash)
		snprintf(buf, sz, "test.c");
	else
		snprintf(buf, sz, "%.*s/test.c", (int)(slash - argv0), argv0);
}

/* Una seccion por BUFFER_SIZE: compila, monta el binario de casos y lo     */
/* lanza. 1 si algo falla.                                                   */
static int	run_bs(const char *repo, const char *self, const t_variant *v,
	const char *bs)
{
	char	dir[4096];
	char	*log;
	pid_t	pid;
	int		st;

	printf("%s── %sBUFFER_SIZE=%s%s\n", C_B, v->tag, bs, C_0);
	log = NULL;
	if (compile_set(repo, v, bs, &log) != 0)
	{
		printf("  %s[COMPILA KO]%s con " CFLAGS " -D BUFFER_SIZE=%s\n", C_KO,
			C_0, bs);
		print_indented(log);
		free(log);
		return (1);
	}
	work_dir(dir, sizeof(dir), v, bs);
	if (sh(&log, "cc -Wall -Wextra -fPIE -pie -DFT_CASES -DFT_BS=%s '%s'"
			" '%s'/*.o -o '%s/cases' 2>&1", bs, self, dir, dir) != 0)
	{
		printf("  %s[TEST KO]%s no se pudo montar el binario de casos\n",
			C_KO, C_0);
		print_indented(log);
		free(log);
		return (1);
	}
	free(log);
	fflush(stdout);
	pid = fork();
	if (pid == 0)
	{
		char	bin[4200];
		char	data[4200];

		snprintf(bin, sizeof(bin), "%s/cases", dir);
		snprintf(data, sizeof(data), "%s/data", dir);
		execl(bin, bin, *v->tag ? "bonus" : "mand", data, (char *)NULL);
		_exit(127);
	}
	waitpid(pid, &st, 0);
	return (!WIFEXITED(st) || WEXITSTATUS(st) != 0);
}

int	main(int argc, char **argv)
{
	const char	*repo;
	char		self[4096];
	int			sections;
	int			ko;
	int			skipped;

	init_colors();
	repo = argc > 1 ? argv[1] : "../repo";
	self_path(self, sizeof(self), argv[0]);
	printf("repo: %s\ntest: %s\n\n", repo, self);
	if (!mkdtemp(g_tmp))
		return (perror("mkdtemp"), 2);
	g_has_norm = system("command -v norminette >/dev/null 2>&1") == 0;
	if (!g_has_norm)
		printf("norminette no esta instalada: no se comprueba la norma\n\n");
	load_libc();
	sections = 0;
	ko = 0;
	skipped = 0;
	for (size_t i = 0; i < N_VAR; i++)
	{
		const t_variant	*v = &g_var[i];

		sections++;
		printf("%s── %s%s\n", C_B, v->name, C_0);
		if (!exists(repo, v->src[0]) && !exists(repo, v->src[1])
			&& !exists(repo, v->hdr))
		{
			printf("  %s[SKIP]%s no hay %s, %s ni %s\n\n", C_SK, C_0,
				v->src[0], v->src[1], v->hdr);
			skipped++;
			continue ;
		}
		ko += check_static(repo, v);
		printf("\n");
		for (size_t b = 0; b < N_BS; b++)
		{
			sections++;
			ko += run_bs(repo, self, v, g_bs[b]);
			printf("\n");
		}
	}
	printf("%s%d ejercicios%s  %s%d con fallos%s  %s%d sin fuente%s\n",
		C_B, sections, C_0, C_KO, ko, C_0, C_SK, skipped, C_0);
	free(g_libc);
	{
		char	cmd[4200];

		snprintf(cmd, sizeof(cmd), "rm -rf '%s'", g_tmp);
		if (system(cmd) != 0)
			fprintf(stderr, "no se pudo borrar %s\n", g_tmp);
	}
	return (ko != 0);
}

#else

/* ================================ CASOS ================================== */

#include <sys/resource.h>

#define W	__attribute__((weak))

W char	*get_next_line(int fd);

/* ------------------------------ andamiaje -------------------------------- */
/* run() lanza cada caso en un hijo con alarm(). El hijo manda el detalle    */
/* del fallo (esperado/obtenido) por una tuberia y sale con 0 si pasa. Los    */
/* casos devuelven 1 (OK) o lo que devuelva ko() (0).                         */

typedef int	(*t_case)(int);

static int	g_fd = -1;
static int	g_nko;

static int	ko(const char *fmt, ...)
{
	char	buf[2048];
	va_list	ap;
	int		n;

	va_start(ap, fmt);
	n = vsnprintf(buf, sizeof(buf) - 1, fmt, ap);
	va_end(ap);
	if (n < 0)
		return (0);
	if ((size_t)n > sizeof(buf) - 2)
		n = sizeof(buf) - 2;
	buf[n++] = '\n';
	if (write(g_fd, buf, (size_t)n) < 0)
		return (0);
	return (0);
}

/* ----------------------------- malloc vigilado --------------------------- */
/* Igual que en el runner de Libft: cabecera con el tamano, G_TAIL bytes de   */
/* guarda detras y el bloque relleno de G_JUNK. g_live cuenta bloques vivos.  */
/* g_nmalloc cuenta siempre; con g_fail_at = k el malloc numero k devuelve    */
/* NULL. El relleno se corta en G_JUNK_MAX: con BUFFER_SIZE=10000000 cada     */
/* llamada reserva 10 MB y rellenarlos enteros se come el timeout.            */

void	*__libc_malloc(size_t n);
void	*__libc_realloc(void *p, size_t n);
void	__libc_free(void *p);

#define G_MAGIC		0x5a454e4755415244ULL
#define G_TAIL		8
#define G_JUNK		0xbe
#define G_JUNK_MAX	65536

typedef struct s_ghdr
{
	size_t				n;
	unsigned long long	magic;
}	t_ghdr;

static long					g_live;
static long					g_fail_at;
static long					g_nmalloc;

static const unsigned char	g_tail[G_TAIL] = {
	0xa5, 0x5a, 0xa5, 0x5a, 0xa5, 0x5a, 0xa5, 0x5a};

static t_ghdr	*ghdr(const void *p)
{
	t_ghdr	*h;

	if (!p)
		return (NULL);
	h = (t_ghdr *)p - 1;
	return (h->magic == G_MAGIC ? h : NULL);
}

static long	tail_hit(const t_ghdr *h)
{
	const unsigned char	*t;

	t = (const unsigned char *)(h + 1) + h->n;
	for (long k = 0; k < G_TAIL; k++)
		if (t[k] != g_tail[k])
			return (k);
	return (-1);
}

static int	guard_ko(const t_ghdr *h)
{
	return (ko("escribe fuera de su memoria: reserva %zu bytes y escribe en"
			" el byte [%zu] (falta sitio para el '\\0'?)", h->n,
			h->n + (size_t)tail_hit(h)));
}

/* La linea devuelta: sin desbordar su bloque y con el '\0' dentro.         */
static int	str_ok(const char *p)
{
	t_ghdr	*h;

	h = ghdr(p);
	if (!h)
		return (1);
	if (tail_hit(h) >= 0)
		return (guard_ko(h));
	if (!memchr(p, '\0', h->n))
		return (ko("la linea no acaba en '\\0': reserva %zu bytes y ninguno"
				" es el terminador", h->n));
	return (1);
}

void	*malloc(size_t n)
{
	t_ghdr	*h;

	g_nmalloc++;
	if ((g_fail_at && g_nmalloc == g_fail_at)
		|| n > SIZE_MAX - sizeof(t_ghdr) - G_TAIL)
	{
		errno = ENOMEM;
		return (NULL);
	}
	h = __libc_malloc(sizeof(t_ghdr) + n + G_TAIL);
	if (!h)
		return (NULL);
	h->n = n;
	h->magic = G_MAGIC;
	g_live++;
	memset(h + 1, G_JUNK, n < G_JUNK_MAX ? n : G_JUNK_MAX);
	memcpy((char *)(h + 1) + n, g_tail, G_TAIL);
	return (h + 1);
}

void	*calloc(size_t nmemb, size_t size)
{
	void	*p;

	if (size && nmemb > SIZE_MAX / size)
	{
		errno = ENOMEM;
		return (NULL);
	}
	p = malloc(nmemb * size);
	if (p)
		memset(p, 0, nmemb * size);
	return (p);
}

void	free(void *p)
{
	t_ghdr	*h;

	if (!p)
		return ;
	h = ghdr(p);
	if (!h)
	{
		__libc_free(p);
		return ;
	}
	if (tail_hit(h) >= 0 && g_fd >= 0)
		_exit(guard_ko(h) ? 0 : 1);
	h->magic = 0;
	g_live--;
	__libc_free(h);
}

void	*realloc(void *p, size_t n)
{
	t_ghdr	*h;
	void	*q;

	if (!p)
		return (malloc(n));
	h = ghdr(p);
	if (!h)
		return (__libc_realloc(p, n));
	if (n == 0)
	{
		free(p);
		return (NULL);
	}
	q = malloc(n);
	if (q)
	{
		memcpy(q, p, h->n < n ? h->n : n);
		free(p);
	}
	return (q);
}

static void	quiet_stderr(void)
{
	int	fd;

	fd = open("/dev/null", O_WRONLY);
	if (fd < 0)
		return ;
	dup2(fd, STDERR_FILENO);
	close(fd);
}

static int	check_leaks(t_case f, int i)
{
	long	base;

	base = g_live;
	if (!f(i))
		return (0);
	if (g_live > base)
		return (ko("deja %ld bloque(s) de memoria sin liberar (leak)",
				g_live - base));
	return (1);
}

static void	run(const char *name, t_case f, int i)
{
	int		fds[2];
	pid_t	pid;
	int		st;
	char	buf[4096];
	ssize_t	r;
	size_t	len;
	char	*ln;

	fflush(stdout);
	if (pipe(fds) != 0)
		return ;
	pid = fork();
	if (pid == 0)
	{
		close(fds[0]);
		g_fd = fds[1];
		quiet_stderr();
		alarm(3);
		_exit(check_leaks(f, i) ? 0 : 1);
	}
	close(fds[1]);
	len = 0;
	while ((r = read(fds[0], buf + len, sizeof(buf) - 1 - len)) > 0)
		len += (size_t)r;
	buf[len] = '\0';
	close(fds[0]);
	waitpid(pid, &st, 0);
	if (!WIFEXITED(st) || WEXITSTATUS(st) != 0)
		g_nko++;
	if (WIFSIGNALED(st))
		printf("  %s[CRASH]%s %s (%s)\n", C_KO, C_0, name,
			WTERMSIG(st) == SIGALRM ? "se cuelga, timeout 3s"
			: strsignal(WTERMSIG(st)));
	else if (WEXITSTATUS(st) == 0)
		printf("  %s[OK]%s   %s\n", C_OK, C_0, name);
	else
	{
		printf("  %s[KO]%s   %s\n", C_KO, C_0, name);
		ln = strtok(buf, "\n");
		while (ln)
		{
			printf("         %s\n", ln);
			ln = strtok(NULL, "\n");
		}
	}
}

/* Representacion legible y recortada de n bytes: "abc\n...(20001 bytes)". */
static const char	*esc(const char *s, size_t n)
{
	static char			bufs[4][256];
	static int			k;
	char				*b;
	size_t				i;
	size_t				o;

	b = bufs[k++ % 4];
	if (!s)
		return ("NULL");
	o = 0;
	b[o++] = '"';
	for (i = 0; i < n && i < 48; i++)
	{
		unsigned char	c = (unsigned char)s[i];

		if (c == '\n')
			o += (size_t)sprintf(b + o, "\\n");
		else if (c == '\t')
			o += (size_t)sprintf(b + o, "\\t");
		else if (c == '\0')
			o += (size_t)sprintf(b + o, "\\0");
		else if (c < 32 || c == 127)
			o += (size_t)sprintf(b + o, "\\x%02x", c);
		else
			b[o++] = (char)c;
	}
	b[o++] = '"';
	b[o] = '\0';
	if (n > 48)
		sprintf(b + o, "...(%zu bytes)", n);
	return (b);
}

/* ------------------------------ ficheros ---------------------------------- */
/* Los genera el propio binario de casos en su directorio de datos antes de  */
/* lanzar nada. FT_BS es el BUFFER_SIZE con el que se ha compilado al        */
/* alumno: las lineas de BS-1, BS y BS+1 bytes son las que pillan los        */
/* off-by-one al cortar el buffer.                                           */

#ifndef FT_BS
# define FT_BS	42
#endif

typedef struct s_file
{
	const char	*name;
	char		*s;
	char		path[4200];
}	t_file;

enum e_file { F_EMPTY, F_ONE_NL, F_ONE, F_NL, F_NLS, F_MULTI, F_MULTI_NONL,
	F_HOLES, F_CHAR, F_UTF8, F_LONG, F_MANY, F_BS, F_MAX };

static t_file	g_files[F_MAX];
static int		g_nfiles;
static char		g_data[4096];

static char	*repeat_lines(void)
{
	char	*s;
	size_t	o;

	s = __libc_malloc(1000 * 130);
	o = 0;
	for (int i = 0; i < 1000; i++)
	{
		int	len = (i * 37) % 121;

		for (int k = 0; k < len; k++)
			s[o++] = (char)('a' + (i + k) % 26);
		s[o++] = '\n';
	}
	s[o] = '\0';
	return (s);
}

static char	*long_line(void)
{
	char	*s;

	s = __libc_malloc(20000 + 8);
	for (int k = 0; k < 20000; k++)
		s[k] = (char)('A' + k % 26);
	strcpy(s + 20000, "\nfin");
	return (s);
}

/* Lineas de BS-1, BS y BS+1 bytes contando el '\n', y una de BS sin '\n'. */
static char	*bs_lines(void)
{
	long	lens[3] = {FT_BS - 1, FT_BS, FT_BS + 1};
	char	*s;
	size_t	o;

	s = __libc_malloc((size_t)FT_BS * 4 + 8);
	o = 0;
	for (int j = 0; j < 3; j++)
	{
		if (lens[j] < 1)
			continue ;
		for (long k = 0; k < lens[j] - 1; k++)
			s[o++] = (char)('x' + j);
		s[o++] = '\n';
	}
	for (long k = 0; k < FT_BS; k++)
		s[o++] = 'w';
	s[o] = '\0';
	return (s);
}

static void	add_file(int id, const char *name, char *s)
{
	t_file	*f;
	int		fd;

	f = &g_files[id];
	f->name = name;
	f->s = s;
	snprintf(f->path, sizeof(f->path), "%s/%02d.txt", g_data, id);
	fd = open(f->path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0 || write(fd, s, strlen(s)) < 0)
		perror(f->path);
	if (fd >= 0)
		close(fd);
	if (id + 1 > g_nfiles)
		g_nfiles = id + 1;
}

static void	build_files(void)
{
	add_file(F_EMPTY, "vacio", "");
	add_file(F_ONE_NL, "una linea con \\n", "hola mundo\n");
	add_file(F_ONE, "una linea sin \\n", "hola mundo");
	add_file(F_NL, "solo un \\n", "\n");
	add_file(F_NLS, "solo saltos de linea", "\n\n\n\n");
	add_file(F_MULTI, "varias lineas", "uno\ndos\ntres\n");
	add_file(F_MULTI_NONL, "varias lineas, la ultima sin \\n",
		"uno\ndos\ntres");
	add_file(F_HOLES, "lineas vacias en medio", "a\n\nb\n\n\nc\n");
	add_file(F_CHAR, "un solo caracter", "x");
	add_file(F_UTF8, "utf-8 y tabuladores",
		"\xc3\xb1" "and\xc3\xba\tping\xc3\xbc" "ino\n\xe2\x82\xac" "uro\n");
	add_file(F_LONG, "una linea de 20000 bytes", long_line());
	add_file(F_MANY, "1000 lineas de 0 a 120 bytes", repeat_lines());
	if (FT_BS <= 100000)
		add_file(F_BS, "lineas de BUFFER_SIZE-1, BUFFER_SIZE y"
			" BUFFER_SIZE+1", bs_lines());
}

static int	open_file(int id)
{
	int	fd;

	fd = open(g_files[id].path, O_RDONLY);
	if (fd < 0)
		ko("no se pudo abrir %s: %s", g_files[id].path, strerror(errno));
	return (fd);
}

/* ------------------------------ comparar ---------------------------------- */

/* Siguiente linea esperada de s a partir de *pos (con su '\n' si lo tiene). */
static const char	*next_line(const char *s, size_t *pos, size_t *len)
{
	const char	*start;
	const char	*nl;

	start = s + *pos;
	if (!*start)
		return (NULL);
	nl = strchr(start, '\n');
	*len = nl ? (size_t)(nl - start) + 1 : strlen(start);
	*pos += *len;
	return (start);
}

/* Compara lo que devuelve get_next_line con lo esperado y lo libera.        */
/* exp NULL = se esperaba el NULL de fin. where dice que llamada era.        */
static int	expect(char *got, const char *exp, size_t elen, const char *where)
{
	size_t	glen;
	size_t	d;

	if (!exp)
	{
		if (!got)
			return (1);
		glen = strlen(got);
		ko("%s: esperaba NULL y devuelve %s", where, esc(got, glen));
		free(got);
		return (0);
	}
	if (!got)
		return (ko("%s: devuelve NULL y esperaba %s", where, esc(exp, elen)));
	if (!str_ok(got))
		return (0);
	glen = strlen(got);
	if (glen == elen && !memcmp(got, exp, elen))
	{
		free(got);
		return (1);
	}
	for (d = 0; d < glen && d < elen && got[d] == exp[d]; d++)
		;
	ko("%s: esperado %s\nobtenido %s (difiere en el byte %zu)", where,
		esc(exp, elen), esc(got, glen), d);
	free(got);
	return (0);
}

/* Lee fd hasta el final contrastando con s, y dos NULL mas tras el final. */
static int	read_all(int fd, const char *s)
{
	char		where[64];
	const char	*e;
	size_t		pos;
	size_t		el;
	int			n;

	pos = 0;
	n = 0;
	while ((e = next_line(s, &pos, &el)))
	{
		snprintf(where, sizeof(where), "linea %d", ++n);
		if (!expect(get_next_line(fd), e, el, where))
			return (0);
	}
	snprintf(where, sizeof(where), "llamada %d (fin)", ++n);
	if (!expect(get_next_line(fd), NULL, 0, where))
		return (0);
	snprintf(where, sizeof(where), "llamada %d (otra vez tras el fin)", ++n);
	return (expect(get_next_line(fd), NULL, 0, where));
}

/* Llama hasta el NULL tirando lo que salga: para vaciar un fd roto.       */
static void	drain(int fd)
{
	char	*s;

	for (int k = 0; k < 5000; k++)
	{
		s = get_next_line(fd);
		if (!s)
			return ;
		free(s);
	}
}

/* --------------------------- casos obligatorios --------------------------- */

static int	c_file(int i)
{
	int	fd;
	int	r;

	if ((fd = open_file(i)) < 0)
		return (0);
	r = read_all(fd, g_files[i].s);
	close(fd);
	return (r);
}

static int	c_stdin(int i)
{
	int	fd;

	if ((fd = open_file(i)) < 0)
		return (0);
	dup2(fd, STDIN_FILENO);
	close(fd);
	return (read_all(STDIN_FILENO, g_files[i].s));
}

/* Un escritor que manda la entrada a trozos y con pausas: cada read de la  */
/* tuberia devuelve menos de lo pedido y a veces corta a media linea.       */
static int	c_pipe(int i)
{
	static const char	*parts[] = {"hola ", "mundo\nque", " tal\n", "", "\n",
		"adios"};
	int					p[2];
	pid_t				pid;
	int					r;

	(void)i;
	if (pipe(p) != 0)
		return (ko("pipe: %s", strerror(errno)));
	pid = fork();
	if (pid == 0)
	{
		close(p[0]);
		for (size_t k = 0; k < sizeof(parts) / sizeof(*parts); k++)
		{
			if (write(p[1], parts[k], strlen(parts[k])) < 0)
				_exit(1);
			usleep(30000);
		}
		_exit(0);
	}
	close(p[1]);
	dup2(p[0], STDIN_FILENO);
	close(p[0]);
	r = read_all(STDIN_FILENO, "hola mundo\nque tal\n\nadios");
	waitpid(pid, NULL, 0);
	return (r);
}

/* 0: -1   1: 4242, sin abrir   2: un fd ya cerrado   3: un directorio     */
static int	c_badfd(int i)
{
	int	fd;

	fd = -1;
	if (i == 1)
		fd = 4242;
	else if (i == 2)
	{
		if ((fd = open_file(F_MULTI)) < 0)
			return (0);
		close(fd);
	}
	else if (i == 3 && (fd = open(g_data, O_RDONLY)) < 0)
		return (ko("no se pudo abrir el directorio %s", g_data));
	if (!expect(get_next_line(fd), NULL, 0, "primera llamada"))
		return (0);
	return (expect(get_next_line(fd), NULL, 0, "segunda llamada"));
}

/* Lee una linea, cierra el fd por debajo (read pasa a dar -1) y vacia. Al  */
/* reabrir, open devuelve el mismo numero de fd: si get_next_line no limpio */
/* lo que guardaba, sale el resto del fichero viejo en vez del principio.   */
static int	c_readerr(int i)
{
	int	fd;
	int	fd2;
	int	r;

	if ((fd = open_file(i)) < 0)
		return (0);
	{
		size_t		pos = 0;
		size_t		el;
		const char	*e = next_line(g_files[i].s, &pos, &el);

		if (!expect(get_next_line(fd), e, el, "linea 1 antes del error"))
			return (close(fd), 0);
	}
	close(fd);
	drain(fd);
	if ((fd2 = open_file(i)) < 0)
		return (0);
	if (fd2 != fd)
		return (close(fd2), ko("el fd reabierto no es el mismo (%d != %d)",
				fd2, fd));
	r = read_all(fd2, g_files[i].s);
	close(fd2);
	return (r);
}

/* Fichero entero, se cierra, y otro en el mismo numero de fd.             */
static int	c_two_files(int i)
{
	int	fd;

	(void)i;
	if ((fd = open_file(F_HOLES)) < 0)
		return (0);
	if (!read_all(fd, g_files[F_HOLES].s))
		return (close(fd), 0);
	close(fd);
	if ((fd = open_file(F_MULTI_NONL)) < 0)
		return (0);
	i = read_all(fd, g_files[F_MULTI_NONL].s);
	close(fd);
	return (i);
}

/* ---------------------------- malloc que falla ---------------------------- */

/* Lee el fichero hasta el primer NULL, que llega antes de tiempo por el     */
/* malloc fallido: lo que salga antes tiene que ser correcto.               */
static int	f_fail(void)
{
	char		where[64];
	char		*got;
	const char	*e;
	size_t		pos;
	size_t		el;
	int			fd;
	int			n;

	if ((fd = open_file(F_MULTI)) < 0)
		return (0);
	pos = 0;
	n = 0;
	while ((got = get_next_line(fd)))
	{
		e = next_line(g_files[F_MULTI].s, &pos, &el);
		snprintf(where, sizeof(where), "linea %d", ++n);
		if (!expect(got, e, el, where))
			return (close(fd), 0);
	}
	close(fd);
	return (1);
}

static int	c_fail(int k)
{
	int	r;

	g_nmalloc = 0;
	g_fail_at = k;
	r = f_fail();
	g_fail_at = 0;
	return (r);
}

/* Cuantos malloc hace una lectura normal, contados en un hijo para no      */
/* dejar el static de get_next_line a medias aqui.                          */
static long	count_mallocs(void)
{
	int		p[2];
	pid_t	pid;
	long	n;

	if (pipe(p) != 0)
		return (0);
	pid = fork();
	if (pid == 0)
	{
		close(p[0]);
		g_fd = open("/dev/null", O_WRONLY);
		alarm(3);
		g_nmalloc = 0;
		f_fail();
		n = g_nmalloc;
		_exit(write(p[1], &n, sizeof(n)) < 0);
	}
	close(p[1]);
	n = 0;
	if (read(p[0], &n, sizeof(n)) != sizeof(n))
		n = 0;
	close(p[0]);
	waitpid(pid, NULL, 0);
	return (n);
}

/* Hasta 12 puntos de fallo repartidos entre el primer malloc y el ultimo. */
static void	fail_sweep(void)
{
	char	nm[160];
	long	n;
	long	m;
	long	k;

	n = count_mallocs();
	if (n <= 0)
		return ;
	m = n < 12 ? n : 12;
	for (long j = 0; j < m; j++)
	{
		k = m == 1 ? 1 : 1 + j * (n - 1) / (m - 1);
		snprintf(nm, sizeof(nm), "varias lineas con el malloc n\xc2\xba %ld"
			" de %ld fallando: NULL y sin leaks", k, n);
		run(nm, c_fail, (int)k);
	}
}

/* ------------------------------ casos bonus ------------------------------- */

/* Varios fds a la vez, una linea de cada uno por turno. Con noise, entre   */
/* turno y turno se llama con fds que no valen: no deben molestar.          */
static int	interleave(const int *fds, const int *ids, int n, int noise)
{
	char		where[64];
	size_t		pos[4] = {0};
	int			nline[4] = {0};
	int			done[4] = {0};
	const char	*e;
	size_t		el;
	int			left;

	left = n;
	while (left)
	{
		for (int k = 0; k < n; k++)
		{
			if (done[k])
				continue ;
			e = next_line(g_files[ids[k]].s, &pos[k], &el);
			snprintf(where, sizeof(where), "fd %d (%s), linea %d", fds[k],
				g_files[ids[k]].name, ++nline[k]);
			if (!expect(get_next_line(fds[k]), e, el, where))
				return (0);
			if (!e && (done[k] = 1))
				left--;
			if (noise && (!expect(get_next_line(-1), NULL, 0, "fd -1 en medio")
					|| !expect(get_next_line(4242), NULL, 0, "fd 4242 en medio")))
				return (0);
		}
	}
	return (1);
}

static int	c_interleave(int noise)
{
	const int	ids[3] = {F_MULTI, F_HOLES, F_MANY};
	int			fds[3];
	int			r;

	for (int k = 0; k < 3; k++)
		if ((fds[k] = open_file(ids[k])) < 0)
			return (0);
	r = interleave(fds, ids, 3, noise);
	for (int k = 0; k < 3; k++)
		close(fds[k]);
	return (r);
}

static int	c_stdin_mix(int i)
{
	const int	ids[2] = {F_MULTI_NONL, F_HOLES};
	int			fds[2];
	int			fd;

	(void)i;
	if ((fd = open_file(ids[0])) < 0)
		return (0);
	dup2(fd, STDIN_FILENO);
	close(fd);
	fds[0] = STDIN_FILENO;
	if ((fds[1] = open_file(ids[1])) < 0)
		return (0);
	i = interleave(fds, ids, 2, 0);
	close(fds[1]);
	return (i);
}

/* A y B a la vez; B se rompe a media lectura y A tiene que seguir igual.   */
static int	c_err_one(int i)
{
	const int	ids[2] = {F_MULTI, F_HOLES};
	int			a;
	int			b;
	size_t		pos;
	size_t		el;
	const char	*e;

	(void)i;
	if ((a = open_file(ids[0])) < 0 || (b = open_file(ids[1])) < 0)
		return (0);
	pos = 0;
	e = next_line(g_files[ids[0]].s, &pos, &el);
	if (!expect(get_next_line(a), e, el, "fd A, linea 1"))
		return (0);
	{
		size_t	pb = 0;

		e = next_line(g_files[ids[1]].s, &pb, &el);
		if (!expect(get_next_line(b), e, el, "fd B, linea 1"))
			return (0);
	}
	close(b);
	drain(b);
	{
		char	where[64];
		int		n = 1;

		while ((e = next_line(g_files[ids[0]].s, &pos, &el)))
		{
			snprintf(where, sizeof(where), "fd A tras romperse B, linea %d",
				++n);
			if (!expect(get_next_line(a), e, el, where))
				return (0);
		}
		if (!expect(get_next_line(a), NULL, 0, "fd A, fin"))
			return (0);
	}
	close(a);
	if ((b = open_file(ids[1])) < 0)
		return (0);
	i = read_all(b, g_files[ids[1]].s);
	close(b);
	return (i);
}

/* Un fd alto: los que guardan el estado en un array de FOPEN_MAX (16) se   */
/* salen de el.                                                              */
static int	c_highfd(int i)
{
	struct rlimit	rl;
	int				fd;
	int				hi;

	(void)i;
	hi = 1000;
	if (getrlimit(RLIMIT_NOFILE, &rl) == 0 && rl.rlim_cur <= (rlim_t)hi)
		hi = (int)rl.rlim_cur - 1;
	if ((fd = open_file(F_MULTI)) < 0)
		return (0);
	if (dup2(fd, hi) < 0)
		return (ko("dup2 a %d: %s", hi, strerror(errno)));
	close(fd);
	i = read_all(hi, g_files[F_MULTI].s);
	close(hi);
	return (i);
}

/* -------------------------------- suites ---------------------------------- */

static void	suite_mand(void)
{
	char	nm[160];

	for (int i = 0; i < g_nfiles; i++)
	{
		if (!g_files[i].name)
			continue ;
		snprintf(nm, sizeof(nm), "fichero: %s", g_files[i].name);
		run(nm, c_file, i);
	}
	run("stdin (fd 0): varias lineas", c_stdin, F_MULTI);
	run("stdin (fd 0): una linea sin \\n", c_stdin, F_ONE);
	run("stdin (fd 0): vacio", c_stdin, F_EMPTY);
	run("stdin desde una tuberia que escribe a trozos", c_pipe, 0);
	run("fd -1: NULL", c_badfd, 0);
	run("fd 4242 sin abrir: NULL", c_badfd, 1);
	run("fd ya cerrado: NULL", c_badfd, 2);
	run("fd de un directorio (read falla): NULL", c_badfd, 3);
	run("read falla tras la linea 1; al reabrir empieza de cero",
		c_readerr, F_MULTI);
	run("read falla en 1000 lineas; al reabrir empieza de cero",
		c_readerr, F_MANY);
	run("un fichero hasta el final y otro en el mismo fd", c_two_files, 0);
	fail_sweep();
}

static void	suite_bonus(void)
{
	suite_mand();
	run("bonus: 3 fds a la vez, una linea de cada uno", c_interleave, 0);
	run("bonus: 3 fds a la vez con fds invalidos en medio", c_interleave, 1);
	run("bonus: stdin y un fichero a la vez", c_stdin_mix, 0);
	run("bonus: un fd se rompe y el otro sigue", c_err_one, 0);
	run("bonus: fd alto (1000)", c_highfd, 0);
}

int	main(int argc, char **argv)
{
	init_colors();
	if (argc != 3)
		return (2);
	if (!get_next_line)
	{
		printf("  %s[KO]%s   no hay get_next_line (revisa el nombre)\n", C_KO,
			C_0);
		return (1);
	}
	snprintf(g_data, sizeof(g_data), "%s", argv[2]);
	mkdir(g_data, 0755);
	build_files();
	if (!strcmp(argv[1], "bonus"))
		suite_bonus();
	else
		suite_mand();
	return (g_nko != 0);
}

#endif
