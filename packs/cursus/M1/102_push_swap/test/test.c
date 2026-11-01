/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para push_swap (42next, v1.0)                   */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*              (o desde test/:  make)                                        */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*              (o desde test/:  make test)                                   */
/*                                                                            */
/*   Todo es de caja negra sobre el binario push_swap:                        */
/*                                                                            */
/*   - push_swap: que esten Makefile, .h y .c, README.md, norminette en       */
/*     todos los .c y .h (libft/ incluida) y libft/ con su Makefile.          */
/*   - Makefile: se prueba en una copia del repo en /tmp (sin .git, .o, .a   */
/*     ni el binario) para no dejar nada en repo/: flags, reglas, cc,        */
/*     libft con su Makefile, sin relink, clean, fclean y re. Con el binario */
/*     que sale: nm para ver que no hay variables globales y que solo llama  */
/*     a read, write, malloc, free y exit.                                    */
/*   - errores: "Error\n" en stderr y nada en stdout; sin argumentos no sale */
/*     nada; los casos limite validos (INT_MIN/INT_MAX, "3 2 1", ordenada,   */
/*     un solo numero).                                                       */
/*   - una seccion por estrategia (--simple, --medium, --complex,            */
/*     --adaptive y sin flag): entradas de 2 a 500 numeros con semillas fijas */
/*     y entradas con poco desorden. La salida tiene que ser solo nombres de  */
/*     operaciones con su '\n' y pasar el checker_linux de extra/.            */
/*   - rendimiento: 100 y 500 aleatorios con varias semillas, sin flag y con  */
/*     --complex, contra los cortes del subject.                              */
/*   - --bench: todo a stderr, stdout igual que sin --bench, desorden con    */
/*     dos decimales (comparado con la formula del subject), estrategia,     */
/*     total y las 11 operaciones (comparados con lo que sale por stdout).   */
/*   - leaks: unos cuantos casos bajo valgrind, si esta instalado.            */
/*                                                                            */
/*   Cada ejecucion va en un hijo con alarm(): un cuelgue es un [CRASH].      */
/*   Anadir un caso de error = una linea en g_errs. Anadir una entrada a las  */
/*   secciones de estrategia = un bloque en sort_section().                   */
/*                                                                            */
/* ************************************************************************** */

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define CFLAGS	"-Wall -Wextra -Werror"
#define NAME	"push_swap"
#define TMO		5		/* segundos por ejecucion de push_swap */
#define TMO_VG	60		/* segundos por ejecucion bajo valgrind */

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

static int		g_has_norm;
static int		g_has_vg;
static char		g_tmp[] = "/tmp/push_swap_runner.XXXXXX";
static char		g_bin[4200];		/* push_swap compilado, "" si no hay */
static char		g_checker[4200];	/* checker_linux, "" si no hay */

/* ============================== utilidades =============================== */

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

/* Errores de norminette de un archivo, una linea por error y sin colores, */
/* cada una con el nombre delante y como mucho NORM_MAX. NULL si pasa.       */
#define NORM_MAX	3

static char	*norm_errors(const char *path, const char *name)
{
	char	cmd[4200];
	char	*out;
	char	*res;
	char	*ln;
	char	*w;
	size_t	len;
	int		st;
	int		n;

	snprintf(cmd, sizeof(cmd), "norminette '%s' 2>&1", path);
	out = capture(cmd, &st);
	res = NULL;
	n = 0;
	ln = strtok(out, "\n");
	while (ln)
	{
		if ((!strncmp(ln, "Error:", 6) || !strncmp(ln, "Error ", 6))
			&& ++n <= NORM_MAX)
		{
			strip_ansi(ln);
			w = ln + 6;
			while (*w == ' ')
				w++;
			for (char *t = w; *t; t++)
				if (*t == '\t')
					*t = ' ';
			len = res ? strlen(res) : 0;
			res = realloc(res, len + strlen(name) + strlen(w) + 4);
			res[len] = '\0';
			strcat(strcat(strcat(strcat(res, name), ": "), w), "\n");
		}
		ln = strtok(NULL, "\n");
	}
	if (n > NORM_MAX)
	{
		len = strlen(res);
		res = realloc(res, len + strlen(name) + 40);
		sprintf(res + len, "%s: (y %d errores mas)\n", name, n - NORM_MAX);
	}
	free(out);
	return (res);
}

/* Ficheros de un tipo en el repo (sin .git), separados por '\n'.          */
static char	*list_files(const char *repo, const char *pat)
{
	char	*out;

	out = NULL;
	sh(&out, "cd '%s' && find . -name .git -prune -o -type f -name '%s' -print"
		" | sed 's|^\\./||' | sort", repo, pat);
	return (out);
}

static int	exists(const char *repo, const char *f)
{
	char	path[4096];

	snprintf(path, sizeof(path), "%s/%s", repo, f);
	return (access(path, F_OK) == 0);
}

/* Representacion legible y recortada de n bytes: "abc\n...(20001 bytes)". */
static const char	*esc(const char *s, size_t n)
{
	static char	bufs[4][512];
	static int	k;
	char		*b;
	size_t		i;
	size_t		o;

	b = bufs[k++ % 4];
	o = 0;
	b[o++] = '"';
	for (i = 0; i < n && i < 60; i++)
	{
		unsigned char	c = (unsigned char)s[i];

		if (c == '\n')
			o += (size_t)sprintf(b + o, "\\n");
		else if (c == '\t')
			o += (size_t)sprintf(b + o, "\\t");
		else if (c == '\0')
			o += (size_t)sprintf(b + o, "\\0");
		else if (c < 32 || c >= 127)
			o += (size_t)sprintf(b + o, "\\x%02x", c);
		else
			b[o++] = (char)c;
	}
	b[o++] = '"';
	b[o] = '\0';
	if (n > 60)
		sprintf(b + o, "...(%zu bytes)", n);
	return (b);
}

/* ============================ ejecutar procesos ========================== */
/* run_av() lanza av[0] en un hijo con la entrada estandar sacada de in (o   */
/* /dev/null) y stdout/stderr en <tag>.out y <tag>.err del temporal, que      */
/* luego se leen a memoria. alarm() sobrevive al execv, asi que un programa  */
/* que se cuelga muere con SIGALRM; uno que no para de escribir, con SIGXFSZ */
/* al pasar de 64 MB.                                                         */

typedef struct s_res
{
	char	*out;
	size_t	olen;
	char	*err;
	size_t	elen;
	int		st;
}	t_res;

static char	*slurp(const char *path, size_t *len)
{
	FILE	*f;
	char	*buf;
	long	sz;

	*len = 0;
	f = fopen(path, "rb");
	if (!f)
		return (calloc(1, 1));
	fseek(f, 0, SEEK_END);
	sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (sz < 0)
		sz = 0;
	buf = malloc((size_t)sz + 1);
	*len = fread(buf, 1, (size_t)sz, f);
	buf[*len] = '\0';
	fclose(f);
	return (buf);
}

static void	res_free(t_res *r)
{
	free(r->out);
	free(r->err);
	memset(r, 0, sizeof(*r));
}

static void	tmp_path(char *buf, size_t sz, const char *tag, const char *ext)
{
	snprintf(buf, sz, "%s/%s.%s", g_tmp, tag, ext);
}

static void	run_av(t_res *r, char **av, const char *tag, const char *in,
	int tmo)
{
	char			of[4200];
	char			ef[4200];
	struct rlimit	rl;
	pid_t			pid;
	int				fd;

	tmp_path(of, sizeof(of), tag, "out");
	tmp_path(ef, sizeof(ef), tag, "err");
	fflush(stdout);
	pid = fork();
	if (pid == 0)
	{
		fd = open(in ? in : "/dev/null", O_RDONLY);
		dup2(fd, STDIN_FILENO);
		fd = open(of, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		dup2(fd, STDOUT_FILENO);
		fd = open(ef, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		dup2(fd, STDERR_FILENO);
		rl.rlim_cur = 64 << 20;
		rl.rlim_max = 64 << 20;
		setrlimit(RLIMIT_FSIZE, &rl);
		alarm((unsigned)tmo);
		execv(av[0], av);
		_exit(127);
	}
	r->st = 0;
	if (pid < 0 || waitpid(pid, &r->st, 0) < 0)
		r->st = 127 << 8;
	r->out = slurp(of, &r->olen);
	r->err = slurp(ef, &r->elen);
}

/* Por que ha muerto un hijo, para la linea de [CRASH].                     */
static const char	*crash_desc(int st, int tmo)
{
	static char	buf[128];

	if (WTERMSIG(st) == SIGALRM)
		snprintf(buf, sizeof(buf), "se cuelga, timeout %ds", tmo);
	else if (WTERMSIG(st) == SIGXFSZ)
		snprintf(buf, sizeof(buf), "escribe sin parar");
	else
		snprintf(buf, sizeof(buf), "%s", strsignal(WTERMSIG(st)));
	return (buf);
}

/* ================================ entradas =============================== */
/* Una entrada son los argumentos tal cual (args) y los numeros que salen de */
/* ellos (v), que son los que usan la simulacion y el desorden.              */

typedef struct s_in
{
	int		na;
	char	**args;
	int		n;
	int		*v;
	char	desc[128];
}	t_in;

static void	in_free(t_in *in)
{
	for (int k = 0; k < in->na; k++)
		free(in->args[k]);
	free(in->args);
	free(in->v);
	memset(in, 0, sizeof(*in));
}

/* Desde argumentos sueltos; cada uno puede llevar varios numeros.          */
static void	in_args(t_in *in, int na, const char **args)
{
	char	*e;
	char	*p;
	long	x;

	memset(in, 0, sizeof(*in));
	in->na = na;
	in->args = calloc((size_t)na + 1, sizeof(char *));
	for (int k = 0; k < na; k++)
	{
		in->args[k] = strdup(args[k]);
		p = in->args[k];
		while (*p)
		{
			x = strtol(p, &e, 10);
			if (e == p)
			{
				p++;
				continue ;
			}
			in->v = realloc(in->v, sizeof(int) * (size_t)(in->n + 1));
			in->v[in->n++] = (int)x;
			p = e;
		}
	}
}

/* Una cadena con los numeros separados por espacios, un argumento cada uno. */
static void	in_str(t_in *in, const char *s)
{
	char		*cp;
	const char	*av[64];
	int			na;

	cp = strdup(s);
	na = 0;
	for (char *w = strtok(cp, " "); w && na < 64; w = strtok(NULL, " "))
		av[na++] = w;
	in_args(in, na, av);
	free(cp);
	snprintf(in->desc, sizeof(in->desc), "%s", s);
}

static void	in_vals(t_in *in, const int *v, int n)
{
	char	buf[16];

	memset(in, 0, sizeof(*in));
	in->na = n;
	in->n = n;
	in->args = calloc((size_t)n + 1, sizeof(char *));
	in->v = malloc(sizeof(int) * (size_t)(n ? n : 1));
	for (int k = 0; k < n; k++)
	{
		snprintf(buf, sizeof(buf), "%d", v[k]);
		in->args[k] = strdup(buf);
		in->v[k] = v[k];
	}
}

/* Generador fijo (xorshift64*): misma semilla, misma entrada siempre.     */
static unsigned long long	g_rs;

static unsigned int	rnd(void)
{
	g_rs ^= g_rs >> 12;
	g_rs ^= g_rs << 25;
	g_rs ^= g_rs >> 27;
	return ((unsigned int)((g_rs * 2685821657736338717ULL) >> 32));
}

static void	seed(unsigned int s)
{
	g_rs = 0x9e3779b97f4a7c15ULL ^ ((unsigned long long)s * 0x2545F4914F6CDD1DULL);
	if (!g_rs)
		g_rs = 1;
	for (int k = 0; k < 8; k++)
		rnd();
}

/* n numeros distintos ordenados, con negativos: base + k*1000 + ruido.     */
static int	*sorted_vals(int n)
{
	int	*v;

	v = malloc(sizeof(int) * (size_t)(n ? n : 1));
	for (int k = 0; k < n; k++)
		v[k] = -n * 500 + k * 1000 + (int)(rnd() % 1000);
	return (v);
}

static void	gen_random(t_in *in, int n, unsigned int s)
{
	int	*v;
	int	j;
	int	t;

	seed(s);
	v = sorted_vals(n);
	for (int k = n - 1; k > 0; k--)
	{
		j = (int)(rnd() % (unsigned int)(k + 1));
		t = v[k];
		v[k] = v[j];
		v[j] = t;
	}
	in_vals(in, v, n);
	free(v);
	snprintf(in->desc, sizeof(in->desc), "%d aleatorios (semilla %u)", n, s);
}

/* Ordenada y con nsw intercambios entre vecinos cercanos: poco desorden.   */
static void	gen_swaps(t_in *in, int n, int nsw, unsigned int s)
{
	int	*v;
	int	i;
	int	j;
	int	t;

	seed(s);
	v = sorted_vals(n);
	for (int k = 0; k < nsw; k++)
	{
		i = (int)(rnd() % (unsigned int)(n - 1));
		j = i + 1 + (int)(rnd() % 3);
		if (j >= n)
			j = n - 1;
		t = v[i];
		v[i] = v[j];
		v[j] = t;
	}
	in_vals(in, v, n);
	free(v);
	snprintf(in->desc, sizeof(in->desc), "%d casi ordenados (%d cambios)", n,
		nsw);
}

/* Ordenada y rotada: los rot ultimos pasan delante.                        */
static void	gen_rot(t_in *in, int n, int rot)
{
	int	*v;
	int	*w;

	seed((unsigned int)(n * 31 + rot));
	v = sorted_vals(n);
	w = malloc(sizeof(int) * (size_t)n);
	for (int k = 0; k < n; k++)
		w[k] = v[(k + n - rot) % n];
	in_vals(in, w, n);
	free(v);
	free(w);
	snprintf(in->desc, sizeof(in->desc), "%d ordenados y rotados %d", n, rot);
}

static void	gen_rev(t_in *in, int n)
{
	int	*v;
	int	*w;

	seed((unsigned int)n);
	v = sorted_vals(n);
	w = malloc(sizeof(int) * (size_t)n);
	for (int k = 0; k < n; k++)
		w[k] = v[n - 1 - k];
	in_vals(in, w, n);
	free(v);
	free(w);
	snprintf(in->desc, sizeof(in->desc), "%d al reves", n);
}

static void	gen_sorted(t_in *in, int n)
{
	int	*v;

	seed((unsigned int)n + 7);
	v = sorted_vals(n);
	in_vals(in, v, n);
	free(v);
	snprintf(in->desc, sizeof(in->desc), "%d ya ordenados", n);
}

/* Los argumentos en una linea, recortada: para los detalles.               */
static const char	*args_str(const t_in *in)
{
	static char	buf[160];
	size_t		o;
	int			k;

	o = 0;
	buf[0] = '\0';
	for (k = 0; k < in->na && o < 70; k++)
	{
		if (strchr(in->args[k], ' ') || !*in->args[k])
			o += (size_t)snprintf(buf + o, sizeof(buf) - o, "%s\"%s\"",
					k ? " " : "", in->args[k]);
		else
			o += (size_t)snprintf(buf + o, sizeof(buf) - o, "%s%s", k ? " " : "",
					in->args[k]);
	}
	if (k < in->na && o < sizeof(buf) - 20)
		snprintf(buf + o, sizeof(buf) - o, " ... (%d argumentos)", in->na);
	return (buf);
}

/* pares al reves y pares totales, igual que compute_disorder del subject. */
static void	disorder(const t_in *in, long *mist, long *pairs)
{
	*mist = 0;
	*pairs = 0;
	for (int i = 0; i < in->n; i++)
		for (int j = i + 1; j < in->n; j++)
		{
			(*pairs)++;
			if (in->v[i] > in->v[j])
				(*mist)++;
		}
}

/* ================================ lanzar ================================= */

/* av = [pre..., push_swap, flags..., args..., NULL]. Memoria nueva.        */
static char	**mk_av(const char **pre, const char **flags, const t_in *in)
{
	char	**av;
	int		n;

	n = 0;
	av = calloc((size_t)(in ? in->na : 0) + 32, sizeof(char *));
	while (pre && *pre)
		av[n++] = (char *)*pre++;
	av[n++] = g_bin;
	while (flags && *flags)
		av[n++] = (char *)*flags++;
	for (int k = 0; in && k < in->na; k++)
		av[n++] = in->args[k];
	return (av);
}

static void	run_ps(t_res *r, const char **flags, const t_in *in)
{
	char	**av;

	av = mk_av(NULL, flags, in);
	run_av(r, av, "ps", NULL, TMO);
	free(av);
}

/* ============================== simulacion =============================== */
/* Pilas con la cima al final del array. Se usan para validar la salida      */
/* linea a linea y para explicar por que no ordena; el veredicto lo da el    */
/* checker_linux de extra/ si esta.                                          */

static const char	*g_ops[11] = {"sa", "sb", "ss", "pa", "pb", "ra", "rb",
	"rr", "rra", "rrb", "rrr"};

typedef struct s_st
{
	int	*a;
	int	na;
	int	*b;
	int	nb;
}	t_st;

static void	st_swap(int *s, int n)
{
	int	t;

	if (n < 2)
		return ;
	t = s[n - 1];
	s[n - 1] = s[n - 2];
	s[n - 2] = t;
}

static void	st_push(int *d, int *nd, int *s, int *ns)
{
	if (*ns == 0)
		return ;
	d[(*nd)++] = s[--(*ns)];
}

/* rotate: la cima pasa al fondo (indice 0).                                */
static void	st_rot(int *s, int n)
{
	int	t;

	if (n < 2)
		return ;
	t = s[n - 1];
	memmove(s + 1, s, sizeof(int) * (size_t)(n - 1));
	s[0] = t;
}

static void	st_rrot(int *s, int n)
{
	int	t;

	if (n < 2)
		return ;
	t = s[0];
	memmove(s, s + 1, sizeof(int) * (size_t)(n - 1));
	s[n - 1] = t;
}

static void	apply(t_st *s, int op)
{
	if (op == 0 || op == 2)
		st_swap(s->a, s->na);
	if (op == 1 || op == 2)
		st_swap(s->b, s->nb);
	if (op == 3)
		st_push(s->a, &s->na, s->b, &s->nb);
	if (op == 4)
		st_push(s->b, &s->nb, s->a, &s->na);
	if (op == 5 || op == 7)
		st_rot(s->a, s->na);
	if (op == 6 || op == 7)
		st_rot(s->b, s->nb);
	if (op == 8 || op == 10)
		st_rrot(s->a, s->na);
	if (op == 9 || op == 10)
		st_rrot(s->b, s->nb);
}

/* Recorre stdout: cada linea tiene que ser una operacion acabada en '\n'.  */
/* Cuenta cada tipo en cnt, deja el total en *nops y aplica las operaciones */
/* a la entrada. 0 si ordena, 1 si la salida no es valida, 2 si no ordena;  */
/* el motivo va a why.                                                      */
static int	simulate(const t_in *in, const t_res *r, int cnt[11], long *nops,
	char *why, size_t wsz)
{
	t_st	s;
	size_t	i;
	size_t	e;
	long	ln;
	int		op;
	int		res;

	memset(cnt, 0, sizeof(int) * 11);
	s.a = malloc(sizeof(int) * (size_t)(in->n + 1));
	s.b = malloc(sizeof(int) * (size_t)(in->n + 1));
	s.na = in->n;
	s.nb = 0;
	for (int k = 0; k < in->n; k++)
		s.a[k] = in->v[in->n - 1 - k];
	res = 0;
	ln = 0;
	for (i = 0; i < r->olen && !res; i = e + 1)
	{
		ln++;
		for (e = i; e < r->olen && r->out[e] != '\n'; e++)
			;
		op = -1;
		for (int k = 0; k < 11 && e < r->olen; k++)
			if (e - i == strlen(g_ops[k]) && !memcmp(r->out + i, g_ops[k], e - i))
				op = k;
		if (op < 0 && e >= r->olen)
			snprintf(why, wsz, "la linea %ld (%s) no acaba en \\n", ln,
				esc(r->out + i, e - i));
		else if (op < 0)
			snprintf(why, wsz, "la linea %ld no es una operacion: %s", ln,
				esc(r->out + i, e - i));
		if (op < 0)
			res = 1;
		else
		{
			cnt[op]++;
			apply(&s, op);
		}
	}
	*nops = res ? 0 : ln;
	if (!res && s.nb)
	{
		snprintf(why, wsz, "b no queda vacia (%d numeros)", s.nb);
		res = 2;
	}
	for (int k = s.na - 1; !res && k > 0; k--)
		if (s.a[k] > s.a[k - 1])
		{
			snprintf(why, wsz, "a no queda ordenada (posicion %d)", s.na - k);
			res = 2;
		}
	if (res == 2 && r->olen == 0)
		snprintf(why, wsz, "no escribe ninguna operacion");
	free(s.a);
	free(s.b);
	return (res);
}

/* El checker con los mismos argumentos y la salida de push_swap por stdin. */
/* 1 si dice "OK\n". *said queda con lo que dice.                           */
static int	checker(const t_in *in, char *said, size_t sz)
{
	char	in_f[4200];
	char	**av;
	t_res	r;
	int		n;
	int		ok;

	tmp_path(in_f, sizeof(in_f), "ps", "out");
	av = calloc((size_t)in->na + 2, sizeof(char *));
	n = 0;
	av[n++] = g_checker;
	for (int k = 0; k < in->na; k++)
		av[n++] = in->args[k];
	run_av(&r, av, "ck", in_f, TMO);
	free(av);
	ok = WIFEXITED(r.st) && r.olen == 3 && !memcmp(r.out, "OK\n", 3);
	snprintf(said, sz, "%s", esc(r.out, r.olen));
	res_free(&r);
	return (ok);
}

/* Lanza push_swap con flags sobre in y mira que ordene: 0 OK, 1 KO, 2      */
/* CRASH. Deja el numero de operaciones en *nops, el recuento por tipo en    */
/* cnt (si no es NULL) y el motivo en why. bench=1 deja pasar el stderr.    */
static int	check_sort(const t_in *in, const char **flags, int bench,
	long *nops, int cnt[11], char *why, size_t wsz)
{
	t_res	r;
	int		mine[11];
	char	said[600];
	char	diag[600];
	int		sim;
	int		res;

	run_ps(&r, flags, in);
	*nops = 0;
	res = 0;
	if (WIFSIGNALED(r.st))
	{
		snprintf(why, wsz, "%s", crash_desc(r.st, TMO));
		res_free(&r);
		return (2);
	}
	sim = simulate(in, &r, cnt ? cnt : mine, nops, diag, sizeof(diag));
	if (!bench && r.elen)
		res = (snprintf(why, wsz, "escribe en stderr: %s", esc(r.err, r.elen)),
				1);
	else if (sim == 1)
		res = (snprintf(why, wsz, "%s", diag), 1);
	else if (*g_checker && !checker(in, said, sizeof(said)))
		res = (snprintf(why, wsz, "el checker dice %s%s%s", said,
					sim ? ": " : "", sim ? diag : ""), 1);
	else if (!*g_checker && sim)
		res = (snprintf(why, wsz, "%s", diag), 1);
	res_free(&r);
	return (res);
}

/* ================================ secciones ============================== */

/* ------------------------------ push_swap -------------------------------- */

static int	section_static(const char *repo)
{
	char	path[4200];
	char	*files;
	char	*miss;
	char	*log;
	char	*err;
	char	*out;
	char	*ln;
	char	*sv;
	size_t	n_err;
	int		n;
	int		bad;

	printf("%s── push_swap%s\n", C_B, C_0);
	miss = NULL;
	out = NULL;
	if (!exists(repo, "Makefile"))
		add_name(&miss, "Makefile");
	files = list_files(repo, "*.h");
	if (!*files)
		add_name(&miss, "ningun .h");
	free(files);
	files = list_files(repo, "*.c");
	if (!*files)
		add_name(&miss, "ningun .c");
	free(files);
	bad = line(!miss, NULL, "hay Makefile, .h y .c");
	if (miss)
		printf("         falta %s\n", miss);
	free(miss);
	if (!exists(repo, "README.md"))
		bad |= line(0, "no hay README.md en la raiz del repo",
				"README.md con la primera linea en cursiva");
	else
	{
		sh(&out, "head -n 1 '%s/README.md' | tr -d '\\r'", repo);
		n = (int)strlen(out);
		while (n > 0 && (out[n - 1] == '\n' || out[n - 1] == ' '))
			out[--n] = '\0';
		bad |= line(n > 2 && (out[0] == '*' || out[0] == '_')
				&& out[n - 1] == out[0], out,
				"README.md con la primera linea en cursiva");
	}
	if (g_has_norm)
	{
		files = list_files(repo, "*.[ch]");
		log = NULL;
		n = 0;
		for (ln = strtok_r(files, "\n", &sv); ln;
			ln = strtok_r(NULL, "\n", &sv))
		{
			snprintf(path, sizeof(path), "%s/%s", repo, ln);
			n++;
			if (!(err = norm_errors(path, ln)))
				continue ;
			n_err = log ? strlen(log) : 0;
			log = realloc(log, n_err + strlen(err) + 1);
			strcpy(log + n_err, err);
			free(err);
		}
		bad |= line(!log, log, "norminette (%d ficheros, libft/ incluida)", n);
		free(log);
		free(files);
	}
	if (exists(repo, "libft"))
		bad |= line(exists(repo, "libft/Makefile"), NULL,
				"libft/ trae su Makefile");
	free(out);
	printf("\n");
	return (bad);
}

/* ------------------------------- Makefile -------------------------------- */

static int	has_rule(const char *d, const char *rule)
{
	char	*out;
	int		st;

	out = NULL;
	st = sh(&out, "make -n -C '%s' '%s' >/dev/null 2>&1", d, rule);
	free(out);
	return (st == 0);
}

/* make dos veces: la segunda no puede tocar ningun .o, .a ni el binario.   */
static int	no_relink(const char *d)
{
	char	*out;
	int		bad;

	out = NULL;
	sh(&out, "touch '%s/.stamp' && make -C '%s' >/dev/null 2>&1; find '%s'"
		" -newer '%s/.stamp' \\( -name '*.o' -o -name '*.a' -o -name '" NAME "'"
		" \\) | sed 's|^%s/||'", d, d, d, d, d);
	bad = line(!*out, out, "make otra vez no recompila ni enlaza nada (sin"
			" relink)");
	free(out);
	return (bad);
}

/* clang mete un memset/memcpy al inicializar structs: si la palabra no     */
/* sale en el fuente, no la ha escrito el alumno.                            */
static int	in_sources(const char *repo, const char *fn)
{
	char	*out;
	int		st;

	out = NULL;
	st = sh(&out, "grep -rqw --include='*.c' --include='*.h' '%s' '%s'", fn,
			repo);
	free(out);
	return (st == 0);
}

/* Con nm: variables globales (o static) en los .o y funciones externas    */
/* del binario fuera de las autorizadas.                                    */
static int	check_syms(const char *repo, const char *d)
{
	static const char	*ok_ext = " read write malloc free exit"
		" __libc_start_main __stack_chk_fail ";
	char				key[600];
	char				*out;
	char				*forbid;
	char				*w;
	int					bad;

	out = NULL;
	sh(&out, "cd '%s' && nm -A $(find . -name '*.o' | sort) 2>/dev/null | awk"
		" '$2 ~ /^[BbDdCGgSs]$/ && $3 !~ /^\\./ { f = $1; sub(/:[^:]*$/, \"\","
		" f); sub(/^\\.\\//, \"\", f); print $3 \" (\" f \")\" }'", d);
	bad = line(!*out, out, "sin variables globales (nm de los .o)");
	sh(&out, "nm -u '%s/" NAME "' 2>/dev/null | awk '$1 == \"U\" { sub(/@.*/,"
		" \"\", $2); print $2 }' | sort -u", d);
	forbid = NULL;
	for (w = strtok(out, "\n"); w; w = strtok(NULL, "\n"))
	{
		snprintf(key, sizeof(key), " %s ", w);
		if (strstr(ok_ext, key))
			continue ;
		if ((!strcmp(w, "memset") || !strcmp(w, "memcpy")
				|| !strcmp(w, "memmove")) && !in_sources(repo, w))
			continue ;
		add_name(&forbid, w);
	}
	bad |= line(!forbid, NULL, "solo llama a read, write, malloc, free y"
			" exit");
	if (forbid)
		printf("         usa %s\n", forbid);
	free(forbid);
	free(out);
	return (bad);
}

/* El Makefile se prueba en una copia del repo (sin .git, .o, .a ni el     */
/* binario). Si make crea push_swap, se guarda en g_tmp/push_swap y g_bin   */
/* apunta ahi; si no, g_bin queda vacio y las demas secciones no se lanzan. */
static int	section_makefile(const char *repo)
{
	static const char	*rules[] = {NAME, "all", "clean", "fclean", "re"};
	char				d[4200];
	char				*out;
	char				*miss;
	int					bad;
	int					st;

	printf("%s── Makefile%s\n", C_B, C_0);
	out = NULL;
	snprintf(d, sizeof(d), "%s/mk", g_tmp);
	if (!exists(repo, "Makefile"))
	{
		printf("  %s[COMPILA KO]%s no hay Makefile\n\n", C_KO, C_0);
		return (1);
	}
	sh(&out, "mkdir -p '%s' && tar -C '%s' --exclude=.git --exclude='*.o'"
		" --exclude='*.a' --exclude='*.swp' --exclude='./" NAME "' -cf - . |"
		" tar -C '%s' -xf - 2>&1", d, repo, d);
	bad = line(sh(&out, "grep -q -- -Wall '%s/Makefile' && grep -q -- -Wextra"
				" '%s/Makefile' && grep -q -- -Werror '%s/Makefile'", d, d, d) == 0,
			NULL, "usa -Wall -Wextra -Werror");
	miss = NULL;
	for (size_t k = 0; k < sizeof(rules) / sizeof(*rules); k++)
		if (!has_rule(d, rules[k]))
			add_name(&miss, rules[k]);
	bad |= line(!miss, NULL, "tiene las reglas " NAME ", all, clean, fclean y"
			" re");
	if (miss)
		printf("         faltan: %s\n", miss);
	free(miss);
	sh(&out, "make -n -C '%s' 2>/dev/null | grep -E '^(gcc|clang|g\\+\\+|c\\+\\+)"
		" ' | head -n 3", d);
	bad |= line(!*out, out, "compila con cc");
	st = sh(&out, "make -C '%s' > '%s/make.log' 2>&1 && test -x '%s/" NAME "'",
			d, g_tmp, d);
	if (st != 0)
	{
		/* Del log solo interesan los diagnosticos: fuera el eco de las      */
		/* ordenes (cc, ar, rm) y las lineas de make que no son el error.     */
		sh(&out, "grep -Ev '^(make(\\[[0-9]+\\])?: (Entering|Leaving)|(cc|gcc|"
			"clang|ar|rm|make) )' '%s/make.log' | sed 's|%s/||g' | head -n 60",
			g_tmp, d);
		printf("  %s[COMPILA KO]%s make no crea " NAME "\n", C_KO, C_0);
		print_indented(out);
		sh(&out, "make -C '%s' fclean >/dev/null 2>&1", d);
		free(out);
		printf("\n");
		return (1);
	}
	printf("  %s[OK]%s   make crea " NAME "\n", C_OK, C_0);
	snprintf(g_bin, sizeof(g_bin), "%s/" NAME, g_tmp);
	sh(&out, "cp '%s/" NAME "' '%s'", d, g_bin);
	if (exists(d, "libft"))
		bad |= line(sh(&out, "test -f '%s/libft/libft.a' && find '%s/libft'"
					" -name '*.o' | grep -q .", d, d) == 0, NULL,
				"make compila libft con su Makefile (libft/libft.a)");
	bad |= no_relink(d);
	bad |= check_syms(repo, d);
	st = sh(&out, "make -C '%s' clean >/dev/null 2>&1; find '%s' -name '*.o' |"
			" sed 's|^%s/||'; test -f '%s/" NAME "'", d, d, d, d);
	bad |= line(!*out && st == 0, *out ? out : "ha borrado " NAME,
			"make clean borra los .o y deja " NAME);
	sh(&out, "make -C '%s' fclean >/dev/null 2>&1; find '%s' \\( -name '*.o'"
		" -o -name '*.a' -o -name '" NAME "' \\) | sed 's|^%s/||'", d, d, d);
	bad |= line(!*out, out, "make fclean no deja ningun .o, .a ni " NAME);
	st = sh(&out, "make -C '%s' re >/dev/null 2>&1 && test -x '%s/" NAME "'", d,
			d);
	bad |= line(st == 0, NULL, "make re vuelve a crear " NAME);
	sh(&out, "make -C '%s' fclean >/dev/null 2>&1", d);
	free(out);
	printf("\n");
	return (bad);
}

/* Sin binario las secciones de comportamiento no tienen nada que lanzar.   */
static int	no_bin(const char *name)
{
	printf("%s── %s%s\n", C_B, name, C_0);
	printf("  %s[TEST KO]%s sin " NAME " compilado no hay casos que lanzar\n\n",
		C_KO, C_0);
	return (1);
}

/* -------------------------------- errores -------------------------------- */

typedef struct s_err
{
	const char	*desc;
	int			na;
	const char	*args[6];
}	t_err;

static const t_err	g_errs[] = {
{"no es un numero", 1, {"one"}},
{"no es un numero (0 one 2 3, del subject)", 5, {"--adaptive", "0", "one",
	"2", "3"}},
{"letras detras de un numero", 2, {"1a", "2"}},
{"un signo solo", 2, {"+", "1"}},
{"un signo solo", 2, {"-", "1"}},
{"signo doble", 2, {"--1", "2"}},
{"signo detras", 2, {"1-", "2"}},
{"argumento vacio", 2, {"", "1"}},
{"mayor que INT_MAX", 2, {"2147483648", "1"}},
{"menor que INT_MIN", 2, {"-2147483649", "1"}},
{"2^32 (en un int se queda en 0)", 2, {"4294967296", "1"}},
{"muy largo", 2, {"99999999999999999999", "1"}},
{"duplicado (3 2 3, del subject)", 4, {"--simple", "3", "2", "3"}},
{"duplicado -0 y 0", 2, {"-0", "0"}},
{"duplicado +1 y 1", 2, {"+1", "1"}},
{"duplicado 01 y 1", 2, {"01", "1"}},
{"duplicado entre \"1 2\" y 1", 2, {"1 2", "1"}},
{"flag desconocido", 3, {"--foo", "2", "1"}},
{"flag casi igual", 3, {"--simplex", "2", "1"}},
{"dos flags de estrategia", 4, {"--simple", "--complex", "2", "1"}},
};

/* Cada argumento entre comillas si hace falta, para el nombre del caso.   */
static const char	*err_args(const t_err *e)
{
	static char	buf[200];
	size_t		o;

	o = 0;
	buf[0] = '\0';
	for (int k = 0; k < e->na && k < 6; k++)
	{
		if (!*e->args[k] || strchr(e->args[k], ' '))
			o += (size_t)snprintf(buf + o, sizeof(buf) - o, "%s\"%s\"",
					k ? " " : "", e->args[k]);
		else
			o += (size_t)snprintf(buf + o, sizeof(buf) - o, "%s%s", k ? " " : "",
					e->args[k]);
	}
	return (buf);
}

static int	err_case(const t_err *e)
{
	const char	*args[6];
	char		why[1200];
	t_in		in;
	t_res		r;
	int			na;

	na = 0;
	for (int k = 0; k < e->na && k < 6; k++)
		args[na++] = e->args[k];
	in_args(&in, na, args);
	run_ps(&r, NULL, &in);
	in_free(&in);
	if (WIFSIGNALED(r.st))
	{
		printf("  %s[CRASH]%s %s: %s (%s)\n", C_KO, C_0, err_args(e), e->desc,
			crash_desc(r.st, TMO));
		res_free(&r);
		return (1);
	}
	why[0] = '\0';
	if (r.olen)
		snprintf(why, sizeof(why), "stdout tiene que quedar vacio y sale %s\n",
			esc(r.out, r.olen));
	if (r.elen != 6 || memcmp(r.err, "Error\n", 6))
		snprintf(why + strlen(why), sizeof(why) - strlen(why), "stderr:"
			" esperado \"Error\\n\", obtenido %s", esc(r.err, r.elen));
	res_free(&r);
	return (line(!*why, why, "%s: %s -> Error", err_args(e), e->desc));
}

/* Un caso valido: sin stderr, pasa el checker y, si zero, sin operaciones. */
static int	valid_case(const char *name, t_in *in, int zero)
{
	char	why[1200];
	long	nops;
	int		res;

	res = check_sort(in, NULL, 0, &nops, NULL, why, sizeof(why));
	if (res == 0 && zero && nops)
		res = (snprintf(why, sizeof(why), "esperaba 0 operaciones y escribe"
					" %ld", nops), 1);
	in_free(in);
	if (res == 2)
	{
		printf("  %s[CRASH]%s %s (%s)\n", C_KO, C_0, name, why);
		return (1);
	}
	return (line(!res, why, "%s", name));
}

static int	section_errors(void)
{
	const char	*sorted[] = {"-5", "0", "3", "42", "100"};
	const char	*limits[] = {"2147483647", "-2147483648", "0"};
	const char	*quoted[] = {"3 2 1"};
	const char	*mixed[] = {"4 1", "3", "-2"};
	const char	*one[] = {"42"};
	t_in		in;
	t_res		r;
	int			bad;

	printf("%s── errores%s\n", C_B, C_0);
	bad = 0;
	run_ps(&r, NULL, NULL);
	if (WIFSIGNALED(r.st))
		bad |= (printf("  %s[CRASH]%s sin argumentos (%s)\n", C_KO, C_0,
					crash_desc(r.st, TMO)), 1);
	else
		bad |= line(!r.olen && !r.elen, r.olen ? esc(r.out, r.olen)
				: esc(r.err, r.elen), "sin argumentos: no escribe nada");
	res_free(&r);
	for (size_t k = 0; k < sizeof(g_errs) / sizeof(*g_errs); k++)
		bad |= err_case(&g_errs[k]);
	in_args(&in, 3, limits);
	bad |= valid_case("acepta INT_MAX e INT_MIN (2147483647 -2147483648 0) y"
			" ordena", &in, 0);
	in_args(&in, 1, quoted);
	bad |= valid_case("\"3 2 1\" en un solo argumento", &in, 0);
	in_args(&in, 3, mixed);
	bad |= valid_case("\"4 1\" 3 -2 (con y sin comillas)", &in, 0);
	in_args(&in, 5, sorted);
	bad |= valid_case("ya ordenada (-5 0 3 42 100): 0 operaciones", &in, 1);
	gen_sorted(&in, 500);
	bad |= valid_case("500 ya ordenados: 0 operaciones", &in, 1);
	in_args(&in, 1, one);
	bad |= valid_case("un solo numero (42): 0 operaciones", &in, 1);
	printf("\n");
	return (bad);
}

/* ------------------------------ estrategias ------------------------------ */

/* Un caso con varias entradas: OK si todas ordenan. Sale el maximo de     */
/* operaciones y, si falla, la entrada que fallo y por que.                 */
typedef struct s_multi
{
	const char	**flags;
	int			bad;		/* 0 OK, 1 KO, 2 CRASH */
	long		max;
	char		why[1400];
}	t_multi;

static void	multi_add(t_multi *m, t_in *in, int show)
{
	char	why[1200];
	long	nops;
	int		res;

	if (m->bad)
	{
		in_free(in);
		return ;
	}
	res = check_sort(in, m->flags, 0, &nops, NULL, why, sizeof(why));
	if (res)
	{
		m->bad = res;
		if (show)
			snprintf(m->why, sizeof(m->why), "con %s: %s", args_str(in), why);
		else
			snprintf(m->why, sizeof(m->why), "%s", why);
	}
	else if (nops > m->max)
		m->max = nops;
	in_free(in);
}

static int	multi_end(t_multi *m, const char *name, int single)
{
	int	bad;

	if (m->bad == 2)
		printf("  %s[CRASH]%s %s (%s)\n", C_KO, C_0, name, m->why);
	else if (m->bad)
		line(0, m->why, "%s", name);
	else
		line(1, NULL, "%s · %s%ld ops", name, single ? "" : "max ", m->max);
	bad = m->bad != 0;
	m->bad = 0;
	m->max = 0;
	m->why[0] = '\0';
	return (bad);
}

static int	one_case(t_multi *m, t_in *in)
{
	char	name[160];

	snprintf(name, sizeof(name), "%s", in->desc);
	multi_add(m, in, 0);
	return (multi_end(m, name, 1));
}

static int	sort_section(const char *title, const char *flag)
{
	static const int	perms[6][3] = {{1, 2, 3}, {1, 3, 2}, {2, 1, 3},
		{2, 3, 1}, {3, 1, 2}, {3, 2, 1}};
	const char			*flags[2];
	t_multi				m;
	t_in				in;
	int					bad;
	int					v[3];

	printf("%s── %s%s\n", C_B, title, C_0);
	flags[0] = flag;
	flags[1] = NULL;
	memset(&m, 0, sizeof(m));
	m.flags = flags;
	bad = 0;
	in_str(&in, "2 1");
	bad |= one_case(&m, &in);
	for (int k = 0; k < 6; k++)
	{
		for (int j = 0; j < 3; j++)
			v[j] = perms[k][j];
		in_vals(&in, v, 3);
		multi_add(&m, &in, 1);
	}
	bad |= multi_end(&m, "3 numeros: las 6 permutaciones", 0);
	for (unsigned int s = 1; s <= 20; s++)
	{
		gen_random(&in, 5, s);
		multi_add(&m, &in, 1);
	}
	bad |= multi_end(&m, "5 numeros: 20 aleatorios", 0);
	for (unsigned int s = 1; s <= 10; s++)
	{
		gen_random(&in, 10, s);
		multi_add(&m, &in, 1);
	}
	bad |= multi_end(&m, "10 numeros: 10 aleatorios", 0);
	gen_random(&in, 100, 1);
	bad |= one_case(&m, &in);
	gen_random(&in, 500, 1);
	bad |= one_case(&m, &in);
	gen_rev(&in, 100);
	bad |= one_case(&m, &in);
	gen_swaps(&in, 100, 4, 2);
	bad |= one_case(&m, &in);
	gen_rot(&in, 100, 5);
	bad |= one_case(&m, &in);
	gen_rot(&in, 100, 50);
	bad |= one_case(&m, &in);
	gen_swaps(&in, 500, 10, 3);
	bad |= one_case(&m, &in);
	in_str(&in, "2147483647 -2147483648 0 -1 1");
	bad |= one_case(&m, &in);
	gen_sorted(&in, 10);
	{
		char	why[1200];
		long	nops;
		int		res;

		res = check_sort(&in, flags, 0, &nops, NULL, why, sizeof(why));
		if (!res && nops)
			res = (snprintf(why, sizeof(why), "esperaba 0 operaciones y escribe"
						" %ld", nops), 1);
		in_free(&in);
		if (res == 2)
			bad |= (printf("  %s[CRASH]%s 10 ya ordenados: 0 operaciones (%s)\n",
						C_KO, C_0, why), 1);
		else
			bad |= line(!res, why, "10 ya ordenados: 0 operaciones");
	}
	printf("\n");
	return (bad);
}

/* ------------------------------ rendimiento ------------------------------ */

typedef struct s_tier
{
	int	n;
	int	lim[3];		/* minimo, bueno, excelente */
}	t_tier;

static int	perf_case(const char *flag, const t_tier *t, int nseeds)
{
	const char	*flags[2];
	char		why[1200];
	char		first[1400];
	const char	*tier;
	t_in		in;
	long		nops;
	long		max;
	long		sum;
	int			res;
	int			worst;
	int			lim;

	flags[0] = flag;
	flags[1] = NULL;
	max = 0;
	sum = 0;
	worst = 0;
	first[0] = '\0';
	for (int s = 1; s <= nseeds && !worst; s++)
	{
		gen_random(&in, t->n, 1000u + (unsigned int)s);
		res = check_sort(&in, flags, 0, &nops, NULL, why, sizeof(why));
		in_free(&in);
		if (res && !worst)
			snprintf(first, sizeof(first), "semilla %d: %s", 1000 + s, why);
		if (res > worst)
			worst = res;
		if (!res && nops > max)
			max = nops;
		sum += nops;
	}
	if (worst == 2)
	{
		printf("  %s[CRASH]%s %s, %d aleatorios (%s)\n", C_KO, C_0,
			flag ? flag : "sin flag", t->n, first);
		return (1);
	}
	if (worst)
		return (line(0, first, "%s, %d aleatorios (%d semillas): no ordena",
				flag ? flag : "sin flag", t->n, nseeds));
	tier = "no llega al minimo";
	lim = t->lim[0];
	for (int k = 0; k < 3; k++)
		if (max < t->lim[k])
		{
			tier = k == 0 ? "minimo" : k == 1 ? "bueno" : "excelente";
			lim = t->lim[k];
		}
	snprintf(why, sizeof(why), "el subject pide menos de %d (minimo), %d (bueno)"
		" o %d (excelente)", t->lim[0], t->lim[1], t->lim[2]);
	return (line(max < t->lim[0], why, "%s, %d aleatorios (%d semillas): max %ld"
			" · media %ld · %s (%s %d)", flag ? flag : "sin flag", t->n, nseeds,
			max, sum / nseeds, tier, max < t->lim[0] ? "<" : ">=", lim));
}

static int	section_perf(void)
{
	static const t_tier	t100 = {100, {2000, 1500, 700}};
	static const t_tier	t500 = {500, {12000, 8000, 5500}};
	int					bad;

	printf("%s── rendimiento%s\n", C_B, C_0);
	bad = 0;
	bad |= perf_case(NULL, &t100, 20);
	bad |= perf_case(NULL, &t500, 10);
	bad |= perf_case("--complex", &t100, 20);
	bad |= perf_case("--complex", &t500, 10);
	printf("\n");
	return (bad);
}

/* -------------------------------- --bench -------------------------------- */

/* Primer "NN.DD%" (o con coma) de s, en centesimas. -1 si no hay.        */
static long	find_pct(const char *s)
{
	const char	*p;
	const char	*q;
	long		v;

	for (p = s; *p; p++)
	{
		if (*p < '0' || *p > '9' || (p > s && p[-1] >= '0' && p[-1] <= '9'))
			continue ;
		v = 0;
		for (q = p; *q >= '0' && *q <= '9'; q++)
			v = v * 10 + (*q - '0');
		if ((*q != '.' && *q != ',') || q[1] < '0' || q[1] > '9' || q[2] < '0'
			|| q[2] > '9' || (q[3] >= '0' && q[3] <= '9'))
			continue ;
		v = v * 100 + (q[1] - '0') * 10 + (q[2] - '0');
		q += 3;
		while (*q == ' ')
			q++;
		if (*q == '%')
			return (v);
	}
	return (-1);
}

static int	is_alnum(char c)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
		|| (c >= '0' && c <= '9') || c == '_');
}

/* El numero que sigue a la palabra w en s ("ra:  3", "ra = 3", "ra 3").   */
/* -1 si no aparece w como palabra suelta seguida de un numero.            */
static long	num_after(const char *s, const char *w, int icase)
{
	size_t		n;
	const char	*p;
	const char	*q;
	long		v;

	n = strlen(w);
	for (p = s; *p; p++)
	{
		if ((icase ? strncasecmp(p, w, n) : strncmp(p, w, n))
			|| (p > s && is_alnum(p[-1])) || (!icase && is_alnum(p[n])))
			continue ;
		q = p + n;
		while (icase && *q && *q != '\n' && (*q < '0' || *q > '9'))
			q++;
		while (*q == ' ' || *q == ':' || *q == '=' || *q == '\t')
			q++;
		if (*q < '0' || *q > '9')
			continue ;
		v = 0;
		while (*q >= '0' && *q <= '9')
			v = v * 10 + (*q++ - '0');
		return (v);
	}
	return (-1);
}

/* strcasestr de cualquiera de las palabras de ws, separadas por '|'.      */
static int	has_any(const char *s, const char *ws)
{
	char	buf[200];

	snprintf(buf, sizeof(buf), "%s", ws);
	for (char *w = strtok(buf, "|"); w; w = strtok(NULL, "|"))
		if (strcasestr(s, w))
			return (1);
	return (0);
}

/* Lanza flags sobre in y deja stdout y stderr en r. 0 si no revienta.     */
static int	bench_run(t_res *r, const char *f1, const char *f2, const t_in *in)
{
	const char	*flags[3];

	flags[0] = f1;
	flags[1] = f2;
	flags[2] = NULL;
	run_ps(r, flags, in);
	return (WIFSIGNALED(r->st) ? 1 : 0);
}

static int	bench_crash(const char *name, t_res *r)
{
	printf("  %s[CRASH]%s %s (%s)\n", C_KO, C_0, name, crash_desc(r->st, TMO));
	res_free(r);
	return (1);
}

/* La linea de s que lleva el '%' (o la primera), para el detalle.         */
static const char	*pct_line(const char *s)
{
	const char	*p;
	const char	*e;

	p = strchr(s, '%');
	if (!p)
		p = s;
	while (p > s && p[-1] != '\n')
		p--;
	e = strchr(p, '\n');
	return (esc(p, e ? (size_t)(e - p) : strlen(p)));
}

/* Desorden de varias entradas: el de stderr tiene que ser el de la formula, */
/* redondeado o truncado a dos decimales.                                    */
static int	bench_disorder(void)
{
	static const char	*lits[] = {"1 3 2", "3 2 1", "1 2 3 5 4", "1 2 3",
		"5 1 2 3 4 6", "2 1"};
	char				why[1200];
	t_in				in;
	t_res				r;
	long				mist;
	long				pairs;
	long				want;
	long				trunc;
	long				got;
	int					crash;

	why[0] = '\0';
	crash = 0;
	for (int k = 0; k < 9 && !*why; k++)
	{
		if (k < 6)
			in_str(&in, lits[k]);
		else if (k == 6)
			gen_random(&in, 100, 5);
		else if (k == 7)
			gen_swaps(&in, 100, 6, 7);
		else
			gen_rot(&in, 100, 30);
		disorder(&in, &mist, &pairs);
		want = pairs ? (mist * 10000 + pairs / 2) / pairs : 0;
		trunc = pairs ? mist * 10000 / pairs : 0;
		if ((crash = bench_run(&r, "--bench", NULL, &in)))
		{
			in_free(&in);
			return (bench_crash("--bench: desorden en % con dos decimales", &r));
		}
		got = find_pct(r.err);
		if (got != want && got != trunc)
			snprintf(why, sizeof(why), "con %s esperaba %ld.%02ld%% y en stderr"
				" %s %s", args_str(&in), want / 100, want % 100, got < 0
				? "no hay ningun NN.DD%:" : "sale", pct_line(r.err));
		res_free(&r);
		in_free(&in);
	}
	return (line(!*why, why, "--bench: desorden en %% con dos decimales (9"
			" entradas, formula del subject)"));
}

/* Nombre y clase de cada estrategia en stderr.                             */
static int	bench_names(void)
{
	static const char	*flag[] = {NULL, "--simple", "--medium", "--complex",
		"--adaptive"};
	static const char	*words[] = {"adaptive|adaptativ", "simple",
		"medium|medio|media|intermedi", "complex|complej",
		"adaptive|adaptativ"};
	char				why[1200];
	t_in				in;
	t_res				r;

	why[0] = '\0';
	for (int k = 0; k < 5 && !*why; k++)
	{
		gen_random(&in, 20, 9);
		if (bench_run(&r, "--bench", flag[k], &in))
		{
			in_free(&in);
			return (bench_crash("--bench: nombre y clase de la estrategia", &r));
		}
		if (!has_any(r.err, words[k]))
			snprintf(why, sizeof(why), "con %s no sale el nombre (%s) en"
				" stderr: %s", flag[k] ? flag[k] : "sin flag", words[k],
				esc(r.err, r.elen));
		else if (!strstr(r.err, "O("))
			snprintf(why, sizeof(why), "con %s no sale la clase (O(...)) en"
				" stderr: %s", flag[k] ? flag[k] : "sin flag", esc(r.err,
					r.elen));
		res_free(&r);
		in_free(&in);
	}
	return (line(!*why, why, "--bench: nombre y clase de cada estrategia"
			" (sin flag, --simple, --medium, --complex, --adaptive)"));
}

/* Total y las 11 operaciones de stderr contra lo que sale por stdout.      */
static int	bench_counts(const char *flag)
{
	char	why[1200];
	char	diag[600];
	char	*miss;
	char	tmp[64];
	int		cnt[11];
	t_in	in;
	t_res	r;
	long	nops;
	long	got;

	gen_random(&in, 100, 5);
	if (bench_run(&r, "--bench", flag, &in))
	{
		in_free(&in);
		return (bench_crash("--bench: total y recuento por operacion", &r));
	}
	why[0] = '\0';
	if (simulate(&in, &r, cnt, &nops, diag, sizeof(diag)) == 1)
		snprintf(why, sizeof(why), "stdout no vale: %s", diag);
	else if ((got = num_after(r.err, "total", 1)) != nops)
		snprintf(why, sizeof(why), "stdout tiene %ld operaciones y en stderr el"
			" total %s%ld", nops, got < 0 ? "no sale: " : "es ", got < 0 ? 0
			: got);
	miss = NULL;
	for (int k = 0; !*why && k < 11; k++)
	{
		got = num_after(r.err, g_ops[k], 0);
		if (got == cnt[k])
			continue ;
		if (got < 0)
			snprintf(tmp, sizeof(tmp), "%s (no sale)", g_ops[k]);
		else
			snprintf(tmp, sizeof(tmp), "%s %ld (en stdout %d)", g_ops[k], got,
				cnt[k]);
		add_name(&miss, tmp);
	}
	if (miss)
		snprintf(why, sizeof(why), "no cuadran: %s", miss);
	free(miss);
	res_free(&r);
	in_free(&in);
	return (line(!*why, why, "--bench%s%s: total y las 11 operaciones cuadran"
			" con stdout", flag ? " " : "", flag ? flag : ""));
}

static int	section_bench(void)
{
	char	why[1200];
	t_in	in;
	t_res	a;
	t_res	b;
	long	nops;
	int		bad;
	int		res;

	printf("%s── --bench%s\n", C_B, C_0);
	bad = 0;
	gen_random(&in, 100, 5);
	if (bench_run(&a, NULL, NULL, &in))
		bad |= bench_crash("sin --bench", &a);
	else
	{
		bad |= line(!a.elen, esc(a.err, a.elen), "sin --bench no escribe nada"
				" en stderr");
		if (bench_run(&b, "--bench", NULL, &in))
			bad |= bench_crash("--bench", &b);
		else
		{
			bad |= line(a.olen == b.olen && !memcmp(a.out, b.out, a.olen),
					b.olen == a.olen ? "el contenido es distinto" : "tiene otro"
					" numero de bytes", "--bench: stdout igual que sin --bench (%zu"
					" bytes)", a.olen);
			bad |= line(b.elen > 0, NULL, "--bench: escribe el informe en"
					" stderr");
			res_free(&b);
		}
		res_free(&a);
	}
	{
		const char	*flags[] = {"--bench", NULL};

		res = check_sort(&in, flags, 1, &nops, NULL, why, sizeof(why));
		if (res == 2)
			bad |= (printf("  %s[CRASH]%s --bench: stdout pasa el checker (%s)\n",
						C_KO, C_0, why), 1);
		else
			bad |= line(!res, why, "--bench: stdout pasa el checker");
	}
	in_free(&in);
	bad |= bench_disorder();
	bad |= bench_names();
	bad |= bench_counts(NULL);
	bad |= bench_counts("--simple");
	printf("\n");
	return (bad);
}

/* --------------------------------- leaks --------------------------------- */

/* Resumen de valgrind: las lineas que importan, sin el "==pid== ".        */
static char	*vg_summary(void)
{
	char	*out;

	out = NULL;
	sh(&out, "grep -E 'in use at exit:|definitely lost:|indirectly lost:|possibly"
		" lost:|still reachable:|Invalid |uninitialised|Mismatched|Syscall param'"
		" '%s/vg.log' | grep -v ': 0 bytes in 0 blocks' | sed 's/^==[0-9]*== *//'"
		" | head -n 4", g_tmp);
	return (out);
}

static int	leak_case(const char *name, const char **flags, int na,
	const char **args)
{
	char	log[4300];
	char	**av;
	t_in	in;
	t_res	r;
	char	*sum;
	int		bad;
	const char	*pre[] = {"/usr/bin/env", "valgrind", "--leak-check=full",
		"--show-leak-kinds=all", "--errors-for-leak-kinds=all",
		"--error-exitcode=42", log, NULL};

	snprintf(log, sizeof(log), "--log-file=%s/vg.log", g_tmp);
	in_args(&in, na, args);
	av = mk_av(pre, flags, na ? &in : NULL);
	run_av(&r, av, "vg", NULL, TMO_VG);
	free(av);
	in_free(&in);
	if (WIFSIGNALED(r.st))
	{
		printf("  %s[CRASH]%s %s (%s)\n", C_KO, C_0, name, crash_desc(r.st,
				TMO_VG));
		res_free(&r);
		return (1);
	}
	sum = vg_summary();
	bad = line(!(WIFEXITED(r.st) && WEXITSTATUS(r.st) == 42), sum, "%s", name);
	free(sum);
	res_free(&r);
	return (bad);
}

static int	section_leaks(void)
{
	const char	*bench[] = {"--bench", NULL};
	const char	*cplx[] = {"--complex", NULL};
	const char	*a_sorted[] = {"1", "2", "3"};
	const char	*a_bad[] = {"1", "2", "one"};
	const char	*a_dup[] = {"3", "2", "3"};
	const char	*a_big[] = {"5", "2147483648"};
	const char	*a_q[] = {"4 1 3", "2", ""};
	const char	*a_fl[] = {"--simple", "--complex", "2", "1"};
	const char	*a_small[] = {"3", "1", "2"};
	t_in		in;
	int			bad;

	printf("%s── leaks%s\n", C_B, C_0);
	if (!g_has_vg)
	{
		printf("  %s[AVISO]%s valgrind no esta instalado: no se miran leaks\n\n",
			C_SK, C_0);
		return (0);
	}
	bad = 0;
	gen_random(&in, 100, 11);
	bad |= leak_case("100 aleatorios (sin flag)", NULL, in.na,
			(const char **)in.args);
	bad |= leak_case("100 aleatorios (--complex)", cplx, in.na,
			(const char **)in.args);
	in_free(&in);
	bad |= leak_case("--bench 3 1 2", bench, 3, a_small);
	bad |= leak_case("ya ordenada (1 2 3)", NULL, 3, a_sorted);
	bad |= leak_case("Error al final (1 2 one)", NULL, 3, a_bad);
	bad |= leak_case("Error por duplicado (3 2 3)", NULL, 3, a_dup);
	bad |= leak_case("Error por overflow (5 2147483648)", NULL, 2, a_big);
	bad |= leak_case("Error por argumento vacio (\"4 1 3\" 2 \"\")", NULL, 3,
			a_q);
	bad |= leak_case("Error por dos flags (--simple --complex 2 1)", NULL, 4,
			a_fl);
	bad |= leak_case("sin argumentos", NULL, 0, NULL);
	printf("\n");
	return (bad);
}

/* ================================= main ================================== */

static void	self_dir(char *buf, size_t sz, const char *argv0)
{
	const char	*slash;

	slash = strrchr(argv0, '/');
	if (!slash)
		snprintf(buf, sz, ".");
	else
		snprintf(buf, sz, "%.*s", (int)(slash - argv0), argv0);
}

/* El checker_linux de extra/: junto al test o junto al repo.               */
static void	find_checker(const char *dir, const char *repo)
{
	snprintf(g_checker, sizeof(g_checker), "%s/../extra/checker_linux", dir);
	if (access(g_checker, X_OK) == 0)
		return ;
	snprintf(g_checker, sizeof(g_checker), "%s/../extra/checker_linux", repo);
	if (access(g_checker, X_OK) == 0)
		return ;
	g_checker[0] = '\0';
}

int	main(int argc, char **argv)
{
	const char	*repo;
	char		dir[4096];
	char		cmd[4200];
	char		*srcs;
	int			sections;
	int			ko;
	int			skipped;

	init_colors();
	repo = argc > 1 ? argv[1] : "../repo";
	self_dir(dir, sizeof(dir), argv[0]);
	printf("repo: %s\ntest: %s/test.c\n\n", repo, dir);
	if (!mkdtemp(g_tmp))
		return (perror("mkdtemp"), 2);
	g_has_norm = system("command -v norminette >/dev/null 2>&1") == 0;
	g_has_vg = system("command -v valgrind >/dev/null 2>&1") == 0;
	if (!g_has_norm)
		printf("norminette no esta instalada: no se comprueba la norma\n\n");
	find_checker(dir, repo);
	if (!*g_checker)
		printf("no esta extra/checker_linux: el runner simula las operaciones"
			" por su cuenta\n\n");
	srcs = list_files(repo, "*.c");
	sections = 11;
	ko = 0;
	skipped = 0;
	if (!*srcs && !exists(repo, "Makefile"))
	{
		printf("%s── push_swap%s\n", C_B, C_0);
		printf("  %s[SKIP]%s no hay Makefile ni ningun .c\n\n", C_SK, C_0);
		sections = 1;
		skipped = 1;
	}
	else
	{
		ko += section_static(repo);
		ko += section_makefile(repo);
		if (*g_bin)
		{
			ko += section_errors();
			ko += sort_section("--simple", "--simple");
			ko += sort_section("--medium", "--medium");
			ko += sort_section("--complex", "--complex");
			ko += sort_section("--adaptive", "--adaptive");
			ko += sort_section("sin-flag", NULL);
			ko += section_perf();
			ko += section_bench();
			ko += section_leaks();
		}
		else
		{
			ko += no_bin("errores");
			ko += no_bin("--simple");
			ko += no_bin("--medium");
			ko += no_bin("--complex");
			ko += no_bin("--adaptive");
			ko += no_bin("sin-flag");
			ko += no_bin("rendimiento");
			ko += no_bin("--bench");
			ko += no_bin("leaks");
		}
	}
	printf("%s%d ejercicios%s  %s%d con fallos%s  %s%d sin fuente%s\n",
		C_B, sections, C_0, C_KO, ko, C_0, C_SK, skipped, C_0);
	free(srcs);
	snprintf(cmd, sizeof(cmd), "rm -rf '%s'", g_tmp);
	if (system(cmd) != 0)
		fprintf(stderr, "no se pudo borrar %s\n", g_tmp);
	return (ko != 0);
}
