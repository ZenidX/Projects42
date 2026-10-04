/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para Libft                                      */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*              (o desde test/:  make)                                        */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*              (o desde test/:  make test)                                   */
/*                                                                            */
/*   Este archivo hace dos papeles:                                           */
/*                                                                            */
/*   a) RUNNER (compilacion normal). Por cada archivo de g_src:               */
/*      - lo compila solo con -Wall -Wextra -Werror contra el libft.h del     */
/*        repo, dejando el .o en un temporal;                                 */
/*      - mira con nm que no defina simbolos globales sin prefijo ft_ (un     */
/*        "memmove" del alumno taparia al de la libc con el que se compara);  */
/*      - con todos los .o buenos se recompila a si mismo con -DFT_CASES y    */
/*        lanza ese binario una vez por funcion.                              */
/*                                                                            */
/*   b) CASOS (-DFT_CASES). Compara cada ft_ con la funcion original (o con   */
/*      una referencia escrita aqui cuando la libc no la tiene: strlcpy,      */
/*      strlcat, strnstr y las de la parte 2). Cada caso corre en un hijo     */
/*      con timeout, asi que un segfault o un bucle infinito no tumban la     */
/*      tanda. Los prototipos son weak: si una funcion aun no existe, el      */
/*      simbolo vale NULL en vez de romper el enlazado.                       */
/*                                                                            */
/*   Anadir una funcion = una entrada en g_src + su prototipo weak + una      */
/*   suite en g_suite.                                                        */
/*                                                                            */
/* ************************************************************************** */

#define _DEFAULT_SOURCE

#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static const char	*g_src[] = {
	"ft_isalpha.c", "ft_isdigit.c", "ft_isalnum.c", "ft_isascii.c",
	"ft_isprint.c", "ft_strlen.c", "ft_memset.c", "ft_bzero.c",
	"ft_memcpy.c", "ft_memmove.c", "ft_strlcpy.c", "ft_strlcat.c",
	"ft_toupper.c", "ft_tolower.c", "ft_strchr.c", "ft_strrchr.c",
	"ft_strncmp.c", "ft_memchr.c", "ft_memcmp.c", "ft_strnstr.c",
	"ft_atoi.c", "ft_calloc.c", "ft_strdup.c",
	"ft_substr.c", "ft_strjoin.c", "ft_strtrim.c", "ft_split.c",
	"ft_itoa.c", "ft_strmapi.c", "ft_striteri.c",
	"ft_putchar_fd.c", "ft_putstr_fd.c", "ft_putendl_fd.c", "ft_putnbr_fd.c",
	"ft_lstnew.c", "ft_lstadd_front.c", "ft_lstsize.c",
	"ft_lstlast.c", "ft_lstadd_back.c", "ft_lstdelone.c",
	"ft_lstclear.c", "ft_lstiter.c", "ft_lstmap.c",
};
#define N_SRC	(sizeof(g_src) / sizeof(*g_src))

enum e_st { ST_MISSING, ST_CC_KO, ST_BAD_SYM, ST_FORBID, ST_DEP, ST_OK };

static enum e_st	g_st[N_SRC];
static char			*g_log[N_SRC];
static char			g_tmp[] = "/tmp/libft_runner.XXXXXX";

/* Funciones externas que el subject autoriza en cada archivo. Por defecto  */
/* ninguna (parte 1: "no deben depender de ninguna funcion externa").       */
static const char	*allowed(const char *src)
{
	static const char	*mem[] = {"ft_calloc.c", "ft_strdup.c",
		"ft_substr.c", "ft_strjoin.c", "ft_strtrim.c", "ft_split.c",
		"ft_itoa.c", "ft_strmapi.c", NULL};

	for (int k = 0; mem[k]; k++)
		if (!strcmp(src, mem[k]))
			return (" malloc free ");
	if (!strncmp(src, "ft_put", 6))
		return (" write ");
	if (!strncmp(src, "ft_lst", 6))
		return (" malloc free ");
	return (" ");
}

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

static void	obj_path(char *buf, size_t sz, const char *src)
{
	snprintf(buf, sz, "%s/%.*s.o", g_tmp, (int)(strlen(src) - 2), src);
}

/* Nombres de simbolo de nm, uno por token, separados y rodeados por ' '    */
/* para poder buscar " nombre " con strstr.                                 */
static char	*nm_names(const char *obj, const char *flags)
{
	char	cmd[4096];
	char	*out;
	char	*res;
	char	*line;
	char	*name;
	int		st;

	snprintf(cmd, sizeof(cmd), "nm %s '%s' 2>/dev/null", flags, obj);
	out = capture(cmd, &st);
	res = calloc(1, strlen(out) + 2);
	res[0] = ' ';
	line = strtok(out, "\n");
	while (line)
	{
		name = strrchr(line, ' ');
		name = name ? name + 1 : line;
		strcat(strcat(res, name), " ");
		line = strtok(NULL, "\n");
	}
	free(out);
	return (res);
}

static void	add_name(char **log, const char *name)
{
	size_t	len;

	len = *log ? strlen(*log) : 0;
	*log = realloc(*log, len + strlen(name) + 3);
	if (len == 0)
		(*log)[0] = '\0';
	strcat(strcat(*log, len ? ", " : ""), name);
}

/* Por cada .o que compila: simbolos definidos sin ft_ y llamadas a         */
/* funciones externas no autorizadas.                                        */
static void	check_symbols(size_t i, const char *obj)
{
	char	*def;
	char	*und;
	char	*name;
	char	key[256];

	def = nm_names(obj, "-g --defined-only");
	name = strtok(def, " ");
	while (name)
	{
		if (strncmp(name, "ft_", 3))
			add_name(&g_log[i], name);
		name = strtok(NULL, " ");
	}
	free(def);
	if (g_log[i])
	{
		g_st[i] = ST_BAD_SYM;
		return ;
	}
	und = nm_names(obj, "-u");
	name = strtok(und, " ");
	while (name)
	{
		snprintf(key, sizeof(key), " %s ", name);
		if (strncmp(name, "ft_", 3) && strncmp(name, "__", 2)
			&& !strstr(allowed(g_src[i]), key))
			add_name(&g_log[i], name);
		name = strtok(NULL, " ");
	}
	free(und);
	g_st[i] = g_log[i] ? ST_FORBID : ST_OK;
}

/* Si un archivo llama a una ft_ que no esta (falta, no compila o ha caido  */
/* por otra comprobacion), el enlazado de todo el binario de casos fallaria. */
/* Se aparta ese archivo, y se repite porque apartar uno puede dejar a       */
/* otros sin dependencia.                                                    */
static void	check_deps(void)
{
	char	obj[4096];
	char	key[256];
	char	*defs;
	char	*und;
	char	*name;
	int		changed;

	changed = 1;
	while (changed)
	{
		changed = 0;
		defs = strdup(" ");
		for (size_t i = 0; i < N_SRC; i++)
		{
			if (g_st[i] != ST_OK)
				continue ;
			obj_path(obj, sizeof(obj), g_src[i]);
			und = nm_names(obj, "-g --defined-only");
			defs = realloc(defs, strlen(defs) + strlen(und) + 1);
			strcat(defs, und + 1);
			free(und);
		}
		for (size_t i = 0; i < N_SRC; i++)
		{
			if (g_st[i] != ST_OK)
				continue ;
			obj_path(obj, sizeof(obj), g_src[i]);
			und = nm_names(obj, "-u");
			name = strtok(und, " ");
			while (name)
			{
				snprintf(key, sizeof(key), " %s ", name);
				if (!strncmp(name, "ft_", 3) && !strstr(defs, key))
				{
					add_name(&g_log[i], name);
					g_st[i] = ST_DEP;
					changed = 1;
				}
				name = strtok(NULL, " ");
			}
			free(und);
		}
		free(defs);
	}
}

static void	compile_all(const char *repo)
{
	char	path[4096];
	char	obj[4096];
	char	cmd[16384];
	int		st;
	size_t	i;

	i = 0;
	while (i < N_SRC)
	{
		snprintf(path, sizeof(path), "%s/%s", repo, g_src[i]);
		obj_path(obj, sizeof(obj), g_src[i]);
		if (access(path, F_OK) != 0)
			g_st[i] = ST_MISSING;
		else
		{
			snprintf(cmd, sizeof(cmd),
				"cc " CFLAGS " -I'%s' -c '%s' -o '%s' 2>&1", repo, path, obj);
			g_log[i] = capture(cmd, &st);
			if (st != 0)
				g_st[i] = ST_CC_KO;
			else
			{
				free(g_log[i]);
				g_log[i] = NULL;
				check_symbols(i, obj);
			}
		}
		i++;
	}
	check_deps();
}

/* Recompila este mismo fuente con -DFT_CASES y los .o que han pasado.       */
static char	*build_cases(const char *self, int *ok)
{
	char	cmd[32768];
	char	obj[4096];
	size_t	len;
	size_t	i;
	int		st;

	len = (size_t)snprintf(cmd, sizeof(cmd),
			"cc -Wall -Wextra -fPIE -pie -DFT_CASES '%s' -o '%s/cases'", self, g_tmp);
	i = 0;
	while (i < N_SRC)
	{
		if (g_st[i] == ST_OK)
		{
			obj_path(obj, sizeof(obj), g_src[i]);
			len += (size_t)snprintf(cmd + len, sizeof(cmd) - len, " '%s'", obj);
		}
		i++;
	}
	snprintf(cmd + len, sizeof(cmd) - len, " 2>&1");
	*ok = 0;
	{
		char	*out = capture(cmd, &st);

		*ok = (st == 0);
		return (out);
	}
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

static int	run_cases(const char *src)
{
	char	fn[256];
	char	bin[4096];
	pid_t	pid;
	int		st;

	snprintf(fn, sizeof(fn), "%.*s", (int)(strlen(src) - 2), src);
	snprintf(bin, sizeof(bin), "%s/cases", g_tmp);
	fflush(stdout);
	pid = fork();
	if (pid == 0)
	{
		execl(bin, bin, fn, (char *)NULL);
		_exit(127);
	}
	waitpid(pid, &st, 0);
	return (WIFEXITED(st) && WEXITSTATUS(st) == 0);
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

int	main(int argc, char **argv)
{
	const char	*repo;
	char		self[4096];
	char		*cases_log;
	int			cases_ok;
	int			ko;
	int			skipped;
	size_t		i;

	init_colors();
	repo = argc > 1 ? argv[1] : "../repo";
	self_path(self, sizeof(self), argv[0]);
	printf("repo: %s\ntest: %s\n\n", repo, self);
	if (!mkdtemp(g_tmp))
		return (perror("mkdtemp"), 2);
	compile_all(repo);
	cases_log = build_cases(self, &cases_ok);
	ko = 0;
	skipped = 0;
	i = 0;
	while (i < N_SRC)
	{
		printf("%s── %s%s\n", C_B, g_src[i], C_0);
		if (g_st[i] == ST_MISSING)
		{
			printf("  %s[SKIP]%s no existe\n", C_SK, C_0);
			skipped++;
		}
		else if (g_st[i] == ST_CC_KO)
		{
			printf("  %s[COMPILA KO]%s con " CFLAGS "\n", C_KO, C_0);
			print_indented(g_log[i]);
			ko++;
		}
		else if (g_st[i] == ST_BAD_SYM)
		{
			printf("  %s[KO]%s   define %s: todo simbolo global debe empezar"
				" por ft_\n", C_KO, C_0, g_log[i]);
			ko++;
		}
		else if (g_st[i] == ST_FORBID)
		{
			printf("  %s[OK]%s   compila con " CFLAGS "\n", C_OK, C_0);
			printf("  %s[KO]%s   usa %s, que el subject no autoriza aqui\n",
				C_KO, C_0, g_log[i]);
			ko++;
		}
		else if (g_st[i] == ST_DEP)
		{
			printf("  %s[OK]%s   compila con " CFLAGS "\n", C_OK, C_0);
			printf("  %s[TEST KO]%s llama a %s, que no esta disponible (falta su"
				" archivo o no pasa sus comprobaciones)\n", C_KO, C_0, g_log[i]);
			ko++;
		}
		else if (!cases_ok)
		{
			printf("  %s[OK]%s   compila con " CFLAGS "\n", C_OK, C_0);
			printf("  %s[TEST KO]%s no se pudo montar el binario de casos\n",
				C_KO, C_0);
			print_indented(cases_log);
			ko++;
		}
		else
		{
			printf("  %s[OK]%s   compila con " CFLAGS "\n", C_OK, C_0);
			if (!run_cases(g_src[i]))
				ko++;
		}
		printf("\n");
		free(g_log[i]);
		i++;
	}
	printf("%s%zu ejercicios%s  %s%d con fallos%s  %s%d sin fuente%s\n",
		C_B, N_SRC, C_0, C_KO, ko, C_0, C_SK, skipped, C_0);
	free(cases_log);
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

#include <ctype.h>

typedef struct s_list
{
	void			*content;
	struct s_list	*next;
}	t_list;

#define W	__attribute__((weak))

W int		ft_isalpha(int c);
W int		ft_isdigit(int c);
W int		ft_isalnum(int c);
W int		ft_isascii(int c);
W int		ft_isprint(int c);
W size_t	ft_strlen(const char *s);
W void		*ft_memset(void *s, int c, size_t n);
W void		ft_bzero(void *s, size_t n);
W void		*ft_memcpy(void *dest, const void *src, size_t n);
W void		*ft_memmove(void *dest, const void *src, size_t n);
W size_t	ft_strlcpy(char *dst, const char *src, size_t size);
W size_t	ft_strlcat(char *dst, const char *src, size_t size);
W int		ft_toupper(int c);
W int		ft_tolower(int c);
W char		*ft_strchr(const char *s, int c);
W char		*ft_strrchr(const char *s, int c);
W int		ft_strncmp(const char *s1, const char *s2, size_t n);
W void		*ft_memchr(const void *s, int c, size_t n);
W int		ft_memcmp(const void *s1, const void *s2, size_t n);
W char		*ft_strnstr(const char *big, const char *little, size_t len);
W int		ft_atoi(const char *nptr);
W void		*ft_calloc(size_t nmemb, size_t size);
W char		*ft_strdup(const char *s);
W char		*ft_substr(char const *s, unsigned int start, size_t len);
W char		*ft_strjoin(char const *s1, char const *s2);
W char		*ft_strtrim(char const *s1, char const *set);
W char		**ft_split(char const *s, char c);
W char		*ft_itoa(int n);
W char		*ft_strmapi(char const *s, char (*f)(unsigned int, char));
W void		ft_striteri(char *s, void (*f)(unsigned int, char *));
W void		ft_putchar_fd(char c, int fd);
W void		ft_putstr_fd(char *s, int fd);
W void		ft_putendl_fd(char *s, int fd);
W void		ft_putnbr_fd(int n, int fd);
W t_list	*ft_lstnew(void *content);
W void		ft_lstadd_front(t_list **lst, t_list *new);
W unsigned int	ft_lstsize(t_list *lst);
W t_list	*ft_lstlast(t_list *lst);
W void		ft_lstadd_back(t_list **lst, t_list *new);
W void		ft_lstdelone(t_list *lst, void (*del)(void *));
W void		ft_lstclear(t_list **lst, void (*del)(void *));
W void		ft_lstiter(t_list *lst, void (*f)(void *));
W t_list	*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *));

/* ------------------------------ andamiaje -------------------------------- */
/* run() lanza cada caso en un hijo con alarm(2). El hijo manda el detalle    */
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

static void	run(const char *name, t_case f, int i)
{
	int		fds[2];
	pid_t	pid;
	int		st;
	char	buf[4096];
	ssize_t	r;
	size_t	len;
	char	*line;

	fflush(stdout);
	if (pipe(fds) != 0)
		return ;
	pid = fork();
	if (pid == 0)
	{
		close(fds[0]);
		g_fd = fds[1];
		alarm(2);
		_exit(f(i) ? 0 : 1);
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
			WTERMSIG(st) == SIGALRM ? "se cuelga, timeout 2s"
			: strsignal(WTERMSIG(st)));
	else if (WEXITSTATUS(st) == 0)
		printf("  %s[OK]%s   %s\n", C_OK, C_0, name);
	else
	{
		printf("  %s[KO]%s   %s\n", C_KO, C_0, name);
		line = strtok(buf, "\n");
		while (line)
		{
			printf("         %s\n", line);
			line = strtok(NULL, "\n");
		}
	}
}

/* Representacion legible de n bytes: "abc\0\xff". Buffers rotativos para   */
/* poder usar dos en el mismo printf.                                         */
static const char	*esc(const void *p, size_t n)
{
	static char			bufs[4][512];
	static int			k;
	char				*b;
	const unsigned char	*s;
	size_t				i;
	size_t				o;

	b = bufs[k++ % 4];
	if (!p)
		return ("NULL");
	s = p;
	o = 0;
	b[o++] = '"';
	i = 0;
	while (i < n && o < sizeof(bufs[0]) - 8)
	{
		if (s[i] == '\0')
			o += (size_t)sprintf(b + o, "\\0");
		else if (s[i] == '\n')
			o += (size_t)sprintf(b + o, "\\n");
		else if (s[i] < 32 || s[i] > 126)
			o += (size_t)sprintf(b + o, "\\x%02x", s[i]);
		else
			b[o++] = (char)s[i];
		i++;
	}
	b[o++] = '"';
	b[o] = '\0';
	return (b);
}

static const char	*esc_s(const char *s)
{
	return (s ? esc(s, strlen(s)) : "NULL");
}

static int	sign(int x)
{
	return ((x > 0) - (x < 0));
}

#define NCASES(a)	((int)(sizeof(a) / sizeof(*(a))))

/* Recorre la tabla de una suite: nombre del caso = fmt aplicado a la fila. */
#define RUN_TABLE(tab, f, ...) do { \
	char	nm_[256]; \
	for (int i_ = 0; i_ < NCASES(tab); i_++) { \
		snprintf(nm_, sizeof(nm_), __VA_ARGS__); \
		run(nm_, f, i_); \
	} \
} while (0)

/* --------------------- referencias que la libc no trae ------------------- */

static size_t	ref_strlcpy(char *d, const char *s, size_t n)
{
	size_t	i;

	i = 0;
	if (n)
	{
		while (i < n - 1 && s[i])
		{
			d[i] = s[i];
			i++;
		}
		d[i] = '\0';
	}
	return (strlen(s));
}

static size_t	ref_strlcat(char *d, const char *s, size_t n)
{
	size_t	dl;
	size_t	i;

	dl = 0;
	while (dl < n && d[dl])
		dl++;
	if (dl == n)
		return (n + strlen(s));
	i = 0;
	while (s[i] && dl + i < n - 1)
	{
		d[dl + i] = s[i];
		i++;
	}
	d[dl + i] = '\0';
	return (dl + strlen(s));
}

static char	*ref_strnstr(const char *big, const char *lit, size_t len)
{
	size_t	l;
	size_t	i;

	l = strlen(lit);
	if (l == 0)
		return ((char *)big);
	i = 0;
	while (big[i] && i + l <= len)
	{
		if (strncmp(big + i, lit, l) == 0)
			return ((char *)big + i);
		i++;
	}
	return (NULL);
}

/* ------------------------------ is / to ---------------------------------- */

static int	(*g_ft1)(int);
static int	(*g_ref1)(int);

static int	c_is_same(int k)
{
	(void)k;
	for (int c = -1; c <= 255; c++)
		if (!g_ft1(c) != !g_ref1(c))
			return (ko("c = %d: esperado %s, obtenido %d", c,
					g_ref1(c) ? "verdadero" : "0", g_ft1(c)));
	return (1);
}

static int	c_is_01(int k)
{
	int	r;

	(void)k;
	for (int c = -1; c <= 255; c++)
	{
		r = g_ft1(c);
		if (r != 0 && r != 1)
			return (ko("c = %d devuelve %d; el subject pide exactamente 1 o 0",
					c, r));
	}
	return (1);
}

static int	c_to_same(int k)
{
	(void)k;
	for (int c = -1; c <= 255; c++)
		if (g_ft1(c) != g_ref1(c))
			return (ko("c = %d: esperado %d, obtenido %d", c, g_ref1(c),
					g_ft1(c)));
	return (1);
}

static void	suite_is(int (*ft)(int), int (*ref)(int), const char *what)
{
	char	nm[128];

	g_ft1 = ft;
	g_ref1 = ref;
	snprintf(nm, sizeof(nm), "igual que %s para c = -1..255", what);
	run(nm, c_is_same, 0);
	run("devuelve exactamente 1 o 0", c_is_01, 0);
}

static int	isascii_ref(int c)
{
	return (c >= 0 && c <= 127);
}

static void	s_isalpha(void) { suite_is(ft_isalpha, isalpha, "isalpha"); }
static void	s_isdigit(void) { suite_is(ft_isdigit, isdigit, "isdigit"); }
static void	s_isalnum(void) { suite_is(ft_isalnum, isalnum, "isalnum"); }
static void	s_isascii(void) { suite_is(ft_isascii, isascii_ref, "isascii"); }
static void	s_isprint(void) { suite_is(ft_isprint, isprint, "isprint"); }

static void	s_toupper(void)
{
	g_ft1 = ft_toupper;
	g_ref1 = toupper;
	run("igual que toupper para c = -1..255", c_to_same, 0);
}

static void	s_tolower(void)
{
	g_ft1 = ft_tolower;
	g_ref1 = tolower;
	run("igual que tolower para c = -1..255", c_to_same, 0);
}

/* -------------------------------- strlen --------------------------------- */

static const char	*g_strs[] = {"", "a", "hola mundo", "\xff\x80z",
	"con\tsalto\n"};

static int	c_strlen(int i)
{
	size_t	r;

	r = ft_strlen(g_strs[i]);
	if (r != strlen(g_strs[i]))
		return (ko("esperado %zu, obtenido %zu", strlen(g_strs[i]), r));
	return (1);
}

static void	s_strlen(void)
{
	RUN_TABLE(g_strs, c_strlen, "ft_strlen(%s)", esc_s(g_strs[i_]));
}

/* --------------------------- memset / bzero ------------------------------ */

static const struct { int c; size_t n; }	g_mset[] = {
	{'x', 4}, {0, 10}, {'x', 0}, {300, 5}, {-1, 3}, {255, 1}};

static int	c_memset(int i)
{
	char	a[10] = "abcdefghi";
	char	b[10] = "abcdefghi";
	void	*r;

	memset(a, g_mset[i].c, g_mset[i].n);
	r = ft_memset(b, g_mset[i].c, g_mset[i].n);
	if (r != b)
		return (ko("no devuelve s"));
	if (memcmp(a, b, 10))
		return (ko("esperado %s", esc(a, 10)), ko("obtenido %s", esc(b, 10)));
	return (1);
}

static void	s_memset(void)
{
	RUN_TABLE(g_mset, c_memset, "ft_memset(buf, %d, %zu)", g_mset[i_].c,
		g_mset[i_].n);
}

static const size_t	g_nbz[] = {0, 1, 5, 10};

static int	c_bzero(int i)
{
	char	a[10] = "abcdefghi";
	char	b[10] = "abcdefghi";

	memset(a, 0, g_nbz[i]);
	ft_bzero(b, g_nbz[i]);
	if (memcmp(a, b, 10))
		return (ko("esperado %s", esc(a, 10)), ko("obtenido %s", esc(b, 10)));
	return (1);
}

static void	s_bzero(void)
{
	RUN_TABLE(g_nbz, c_bzero, "ft_bzero(buf, %zu)", g_nbz[i_]);
}

/* --------------------------- memcpy / memmove ---------------------------- */

static const size_t	g_ncpy[] = {0, 1, 5, 10};

static int	c_memcpy(int i)
{
	const char	s[10] = "\xff\x80zy\0xwvu";
	char		a[10] = "abcdefghi";
	char		b[10] = "abcdefghi";
	void		*r;

	memcpy(a, s, g_ncpy[i]);
	r = ft_memcpy(b, s, g_ncpy[i]);
	if (r != b)
		return (ko("no devuelve dest"));
	if (memcmp(a, b, 10))
		return (ko("esperado %s", esc(a, 10)), ko("obtenido %s", esc(b, 10)));
	return (1);
}

static void	s_memcpy(void)
{
	RUN_TABLE(g_ncpy, c_memcpy,
		"ft_memcpy(dst, \"\\xff\\x80zy\\0xwvu\", %zu)", g_ncpy[i_]);
}

static const struct { int d; int s; size_t n; }	g_mmv[] = {
	{2, 0, 4}, {0, 2, 4}, {1, 0, 9}, {0, 1, 9}, {3, 3, 5}, {0, 10, 5},
	{10, 0, 5}, {2, 0, 0}};

static int	c_memmove(int i)
{
	char	a[20] = "abcdefghijklmnopqrs";
	char	b[20] = "abcdefghijklmnopqrs";
	void	*r;

	memmove(a + g_mmv[i].d, a + g_mmv[i].s, g_mmv[i].n);
	r = ft_memmove(b + g_mmv[i].d, b + g_mmv[i].s, g_mmv[i].n);
	if (r != b + g_mmv[i].d)
		return (ko("no devuelve dest"));
	if (memcmp(a, b, 20))
		return (ko("esperado %s", esc(a, 19)), ko("obtenido %s", esc(b, 19)));
	return (1);
}

static void	s_memmove(void)
{
	RUN_TABLE(g_mmv, c_memmove, "ft_memmove(str + %d, str + %d, %zu)%s",
		g_mmv[i_].d, g_mmv[i_].s, g_mmv[i_].n,
		g_mmv[i_].d > g_mmv[i_].s && g_mmv[i_].d < g_mmv[i_].s
		+ (int)g_mmv[i_].n ? "  [solapa, dest detras]"
		: g_mmv[i_].s > g_mmv[i_].d && g_mmv[i_].s < g_mmv[i_].d
		+ (int)g_mmv[i_].n ? "  [solapa, dest delante]" : "");
}

/* --------------------------- strlcpy / strlcat --------------------------- */

static const struct { const char *s; size_t n; }	g_lcpy[] = {
	{"hello", 10}, {"hello", 3}, {"long string", 5}, {"", 10}, {"abc", 1},
	{"12345", 5}, {"hello", 0}};

static int	c_strlcpy(int i)
{
	char	a[20] = "XXXXXXXXXXXXXXXXXXX";
	char	b[20] = "XXXXXXXXXXXXXXXXXXX";
	size_t	r1;
	size_t	r2;

	r1 = ref_strlcpy(a, g_lcpy[i].s, g_lcpy[i].n);
	r2 = ft_strlcpy(b, g_lcpy[i].s, g_lcpy[i].n);
	if (r1 != r2 || memcmp(a, b, 20))
		return (ko("esperado ret=%zu dst=%s", r1, esc(a, 19)),
			ko("obtenido ret=%zu dst=%s", r2, esc(b, 19)));
	return (1);
}

static void	s_strlcpy(void)
{
	RUN_TABLE(g_lcpy, c_strlcpy, "ft_strlcpy(dst, %s, %zu)",
		esc_s(g_lcpy[i_].s), g_lcpy[i_].n);
}

static const struct { const char *d; const char *s; size_t n; }	g_lcat[] = {
	{"hola ", "mundo", 20}, {"hola ", "mundo", 11}, {"hola ", "mundo", 10},
	{"hola ", "mundo", 6}, {"hola ", "mundo", 5}, {"hola ", "mundo", 3},
	{"hola ", "mundo", 0}, {"ab", "cdefgh", 5}, {"ab", "cdefgh", 1},
	{"", "abc", 4}, {"abc", "", 10}};

static int	c_strlcat(int i)
{
	char	a[21] = "XXXXXXXXXXXXXXXXXXXX";
	char	b[21] = "XXXXXXXXXXXXXXXXXXXX";
	size_t	r1;
	size_t	r2;

	memcpy(a, g_lcat[i].d, strlen(g_lcat[i].d) + 1);
	memcpy(b, g_lcat[i].d, strlen(g_lcat[i].d) + 1);
	r1 = ref_strlcat(a, g_lcat[i].s, g_lcat[i].n);
	r2 = ft_strlcat(b, g_lcat[i].s, g_lcat[i].n);
	if (r1 != r2 || memcmp(a, b, 20))
		return (ko("esperado ret=%zu dst=%s", r1, esc(a, 20)),
			ko("obtenido ret=%zu dst=%s", r2, esc(b, 20)));
	return (1);
}

static void	s_strlcat(void)
{
	RUN_TABLE(g_lcat, c_strlcat, "ft_strlcat(%s, %s, %zu)",
		esc_s(g_lcat[i_].d), esc_s(g_lcat[i_].s), g_lcat[i_].n);
}

/* --------------------------- strchr / strrchr ---------------------------- */

static const struct { const char *s; int c; }	g_chr[] = {
	{"hola mundo", 'o'}, {"hola mundo", 'h'}, {"hola mundo", 'z'},
	{"hola mundo", '\0'}, {"hola mundo", 'l' + 256}, {"", 'a'}, {"", '\0'}};

static int	c_chr(int i, char *(*ft)(const char *, int),
	char *(*ref)(const char *, int))
{
	const char	*s;
	char		*r1;
	char		*r2;

	s = g_chr[i].s;
	r1 = ref(s, g_chr[i].c);
	r2 = ft(s, g_chr[i].c);
	if (r1 != r2)
	{
		if (r1)
			ko("esperado s + %td", r1 - s);
		else
			ko("esperado NULL");
		if (r2 && r2 >= s && r2 <= s + strlen(s))
			return (ko("obtenido s + %td", r2 - s));
		return (ko("obtenido %s", r2 ? "un puntero fuera de s" : "NULL"));
	}
	return (1);
}

static int	c_strchr(int i) { return (c_chr(i, ft_strchr, strchr)); }
static int	c_strrchr(int i) { return (c_chr(i, ft_strrchr, strrchr)); }

static void	s_strchr(void)
{
	RUN_TABLE(g_chr, c_strchr, "ft_strchr(%s, %d)", esc_s(g_chr[i_].s),
		g_chr[i_].c);
}

static void	s_strrchr(void)
{
	RUN_TABLE(g_chr, c_strrchr, "ft_strrchr(%s, %d)", esc_s(g_chr[i_].s),
		g_chr[i_].c);
}

/* --------------------------- strncmp / memcmp ---------------------------- */

static const struct { const char *a; const char *b; size_t n; }	g_cmp[] = {
	{"abc", "abc", 3}, {"abc", "abd", 3}, {"abc", "abd", 2}, {"abc", "ab", 3},
	{"ab", "abc", 3}, {"", "", 1}, {"a", "b", 0}, {"\x80", "\x01", 1},
	{"abc", "abcde", 10}, {"abc\0x", "abc\0y", 5}};

static int	c_strncmp(int i)
{
	int	r1;
	int	r2;

	r1 = strncmp(g_cmp[i].a, g_cmp[i].b, g_cmp[i].n);
	r2 = ft_strncmp(g_cmp[i].a, g_cmp[i].b, g_cmp[i].n);
	if (sign(r1) != sign(r2))
		return (ko("esperado un valor %s, obtenido %d",
				r1 > 0 ? "> 0" : r1 < 0 ? "< 0" : "== 0", r2));
	return (1);
}

static void	s_strncmp(void)
{
	RUN_TABLE(g_cmp, c_strncmp, "ft_strncmp(%s, %s, %zu)", esc_s(g_cmp[i_].a),
		esc_s(g_cmp[i_].b), g_cmp[i_].n);
}

static const struct { const char *a; const char *b; size_t n; }	g_mcmp[] = {
	{"abc", "abc", 3}, {"abc", "abd", 3}, {"abc", "abd", 2},
	{"\x80", "\x01", 1}, {"ab\0x", "ab\0y", 4}, {"a", "b", 0}};

static int	c_memcmp(int i)
{
	int	r1;
	int	r2;

	r1 = memcmp(g_mcmp[i].a, g_mcmp[i].b, g_mcmp[i].n);
	r2 = ft_memcmp(g_mcmp[i].a, g_mcmp[i].b, g_mcmp[i].n);
	if (sign(r1) != sign(r2))
		return (ko("esperado un valor %s, obtenido %d",
				r1 > 0 ? "> 0" : r1 < 0 ? "< 0" : "== 0", r2));
	return (1);
}

static void	s_memcmp(void)
{
	RUN_TABLE(g_mcmp, c_memcmp, "ft_memcmp(%s, %s, %zu)",
		esc(g_mcmp[i_].a, g_mcmp[i_].n ? g_mcmp[i_].n : 1),
		esc(g_mcmp[i_].b, g_mcmp[i_].n ? g_mcmp[i_].n : 1), g_mcmp[i_].n);
}

/* --------------------------------- memchr -------------------------------- */

static const char	g_mchr_s[] = "hola\0mundo";
static const struct { int c; size_t n; }	g_mchr[] = {
	{'m', 10}, {'z', 10}, {'h', 0}, {'o' + 256, 10}, {'\0', 10}, {'o', 2}};

static int	c_memchr(int i)
{
	const void	*r1;
	const void	*r2;

	r1 = memchr(g_mchr_s, g_mchr[i].c, g_mchr[i].n);
	r2 = ft_memchr(g_mchr_s, g_mchr[i].c, g_mchr[i].n);
	if (r1 != r2)
	{
		if (r1)
			ko("esperado s + %td", (const char *)r1 - g_mchr_s);
		else
			ko("esperado NULL");
		if (r2)
			return (ko("obtenido s + %td", (const char *)r2 - g_mchr_s));
		return (ko("obtenido NULL"));
	}
	return (1);
}

static void	s_memchr(void)
{
	RUN_TABLE(g_mchr, c_memchr, "ft_memchr(\"hola\\0mundo\", %d, %zu)",
		g_mchr[i_].c, g_mchr[i_].n);
}

/* -------------------------------- strnstr -------------------------------- */

static const struct { const char *b; const char *l; size_t n; }	g_nstr[] = {
	{"hola mundo", "mundo", 10}, {"hola mundo", "mundo", 9},
	{"hola", "", 4}, {"hola", "", 0}, {"hola", "hola mundo", 20},
	{"aaab", "aab", 4}, {"hola", "la", 0}, {"", "a", 5}};

static int	c_strnstr(int i)
{
	const char	*b;
	char		*r1;
	char		*r2;

	b = g_nstr[i].b;
	r1 = ref_strnstr(b, g_nstr[i].l, g_nstr[i].n);
	r2 = ft_strnstr(b, g_nstr[i].l, g_nstr[i].n);
	if (r1 != r2)
	{
		if (r1)
			ko("esperado big + %td", r1 - b);
		else
			ko("esperado NULL");
		if (r2)
			return (ko("obtenido big + %td", r2 - b));
		return (ko("obtenido NULL"));
	}
	return (1);
}

static void	s_strnstr(void)
{
	RUN_TABLE(g_nstr, c_strnstr, "ft_strnstr(%s, %s, %zu)",
		esc_s(g_nstr[i_].b), esc_s(g_nstr[i_].l), g_nstr[i_].n);
}

/* --------------------------------- atoi ---------------------------------- */

static const char	*g_atoi[] = {"42", "   -42", "\t\n\v\f\r +7", "+-5", "--5",
	"12abc", "abc", "2147483647", "-2147483648", "0", "", " - 5", "007"};

static int	c_atoi(int i)
{
	int	r1;
	int	r2;

	r1 = atoi(g_atoi[i]);
	r2 = ft_atoi(g_atoi[i]);
	if (r1 != r2)
		return (ko("esperado %d, obtenido %d", r1, r2));
	return (1);
}

static void	s_atoi(void)
{
	RUN_TABLE(g_atoi, c_atoi, "ft_atoi(%s)", esc_s(g_atoi[i_]));
}

/* --------------------------- calloc / strdup ----------------------------- */

static int	c_calloc(int i)
{
	unsigned char	*p;

	if (i == 1)
	{
		p = ft_calloc(SIZE_MAX, 2);
		if (p)
			return (ko("nmemb * size desborda: esperado NULL"));
		return (1);
	}
	p = ft_calloc(5, sizeof(int));
	if (!p)
		return (ko("devuelve NULL"));
	for (size_t k = 0; k < 5 * sizeof(int); k++)
		if (p[k])
			return (ko("el byte %zu no es 0", k));
	free(p);
	return (1);
}

static void	s_calloc(void)
{
	run("ft_calloc(5, sizeof(int)) reserva y pone a 0", c_calloc, 0);
	run("ft_calloc(SIZE_MAX, 2) devuelve NULL (desbordamiento)", c_calloc, 1);
}

static const char	*g_dup[] = {"hola mundo", "", "\xff\x80z"};

static int	c_strdup(int i)
{
	char	*r;

	r = ft_strdup(g_dup[i]);
	if (!r)
		return (ko("devuelve NULL"));
	if (r == g_dup[i])
		return (ko("devuelve el mismo puntero, no una copia"));
	if (strcmp(r, g_dup[i]))
		return (ko("esperado %s", esc_s(g_dup[i])), ko("obtenido %s",
				esc_s(r)));
	free(r);
	return (1);
}

static void	s_strdup(void)
{
	RUN_TABLE(g_dup, c_strdup, "ft_strdup(%s)", esc_s(g_dup[i_]));
}

/* ------------------------- parte 2: cadenas nuevas ----------------------- */

static int	cmp_new(const char *got, const char *exp)
{
	if (!got)
		return (ko("esperado %s", esc_s(exp)), ko("obtenido NULL"));
	if (strcmp(got, exp))
		return (ko("esperado %s", esc_s(exp)), ko("obtenido %s", esc_s(got)));
	return (1);
}

static const struct { const char *s; unsigned int st; size_t n; const char *e; }
	g_sub[] = {
	{"hola mundo", 5, 5, "mundo"}, {"hola", 0, 10, "hola"},
	{"hola", 10, 2, ""}, {"hola", 2, 0, ""}, {"hola", 4, 1, ""},
	{"hola mundo", 1, 3, "ola"}};

static int	c_substr(int i)
{
	return (cmp_new(ft_substr(g_sub[i].s, g_sub[i].st, g_sub[i].n),
			g_sub[i].e));
}

static void	s_substr(void)
{
	RUN_TABLE(g_sub, c_substr, "ft_substr(%s, %u, %zu)", esc_s(g_sub[i_].s),
		g_sub[i_].st, g_sub[i_].n);
}

static const struct { const char *a; const char *b; const char *e; }
	g_join[] = {{"hola ", "mundo", "hola mundo"}, {"", "", ""},
	{"a", "", "a"}, {"", "b", "b"}};

static int	c_strjoin(int i)
{
	return (cmp_new(ft_strjoin(g_join[i].a, g_join[i].b), g_join[i].e));
}

static void	s_strjoin(void)
{
	RUN_TABLE(g_join, c_strjoin, "ft_strjoin(%s, %s)", esc_s(g_join[i_].a),
		esc_s(g_join[i_].b));
}

static const struct { const char *s; const char *set; const char *e; }
	g_trim[] = {{"  hola  ", " ", "hola"}, {"xxhxx", "x", "h"},
	{"xxx", "x", ""}, {"hola", "", "hola"}, {"", "x", ""},
	{"ab hola ba", "ab ", "hol"}, {"  ho la  ", " ", "ho la"}};

static int	c_strtrim(int i)
{
	return (cmp_new(ft_strtrim(g_trim[i].s, g_trim[i].set), g_trim[i].e));
}

static void	s_strtrim(void)
{
	RUN_TABLE(g_trim, c_strtrim, "ft_strtrim(%s, %s)", esc_s(g_trim[i_].s),
		esc_s(g_trim[i_].set));
}

static const struct { const char *s; char c; const char *e[5]; }	g_split[] = {
	{"  hola  que tal ", ' ', {"hola", "que", "tal", NULL}},
	{"", ' ', {NULL}}, {"   ", ' ', {NULL}}, {"hola", ' ', {"hola", NULL}},
	{"a,b,,c", ',', {"a", "b", "c", NULL}}, {"hola", '\0', {"hola", NULL}}};

static int	c_split(int i)
{
	char	**r;
	int		k;

	r = ft_split(g_split[i].s, g_split[i].c);
	if (!r)
		return (ko("devuelve NULL"));
	k = 0;
	while (g_split[i].e[k] || r[k])
	{
		if (!g_split[i].e[k])
			return (ko("sobra la palabra [%d] = %s", k, esc_s(r[k])));
		if (!r[k])
			return (ko("falta la palabra [%d] = %s (el array acaba antes)", k,
					esc_s(g_split[i].e[k])));
		if (strcmp(r[k], g_split[i].e[k]))
			return (ko("palabra [%d]: esperado %s", k, esc_s(g_split[i].e[k])),
				ko("palabra [%d]: obtenido %s", k, esc_s(r[k])));
		k++;
	}
	return (1);
}

static void	s_split(void)
{
	RUN_TABLE(g_split, c_split, "ft_split(%s, '%s')", esc_s(g_split[i_].s),
		g_split[i_].c ? (char [2]){g_split[i_].c, 0} : "\\0");
}

static const int	g_itoa[] = {0, 42, -42, 7, INT_MAX, INT_MIN, -1};

static int	c_itoa(int i)
{
	char	e[16];

	snprintf(e, sizeof(e), "%d", g_itoa[i]);
	return (cmp_new(ft_itoa(g_itoa[i]), e));
}

static void	s_itoa(void)
{
	RUN_TABLE(g_itoa, c_itoa, "ft_itoa(%d)", g_itoa[i_]);
}

/* Mayuscula en los indices pares: asi se nota si el indice va mal.         */
static char	map_even_up(unsigned int i, char c)
{
	return (i % 2 == 0 ? (char)toupper((unsigned char)c) : c);
}

static void	iter_even_up(unsigned int i, char *c)
{
	*c = map_even_up(i, *c);
}

static int	c_strmapi(int i)
{
	if (i == 1)
		return (cmp_new(ft_strmapi("", map_even_up), ""));
	return (cmp_new(ft_strmapi("hola mundo", map_even_up), "HoLa mUnDo"));
}

static void	s_strmapi(void)
{
	run("ft_strmapi(\"hola mundo\", f) con f = mayuscula en indices pares",
		c_strmapi, 0);
	run("ft_strmapi(\"\", f)", c_strmapi, 1);
}

static int	c_striteri(int i)
{
	char	s[] = "hola mundo";

	(void)i;
	ft_striteri(s, iter_even_up);
	return (cmp_new(s, "HoLa mUnDo"));
}

static void	s_striteri(void)
{
	run("ft_striteri(\"hola mundo\", f) con f = mayuscula en indices pares",
		c_striteri, 0);
}

/* ------------------------------ put*_fd ---------------------------------- */
/* Se escribe en una tuberia y se compara lo leido.                          */

static int	check_fd_out(void (*call)(int, int), int arg, const char *exp)
{
	int		p[2];
	char	buf[256];
	ssize_t	r;
	size_t	len;

	if (pipe(p) != 0)
		return (ko("pipe fallo"));
	call(p[1], arg);
	close(p[1]);
	len = 0;
	while ((r = read(p[0], buf + len, sizeof(buf) - 1 - len)) > 0)
		len += (size_t)r;
	buf[len] = '\0';
	close(p[0]);
	if (len != strlen(exp) || memcmp(buf, exp, len))
		return (ko("esperado %s", esc_s(exp)), ko("obtenido %s", esc(buf, len)));
	return (1);
}

static void	call_putchar(int fd, int a) { ft_putchar_fd((char)a, fd); }
static void	call_putstr(int fd, int a)
{
	ft_putstr_fd(a ? "hola mundo" : "", fd);
}
static void	call_putendl(int fd, int a)
{
	ft_putendl_fd(a ? "hola" : "", fd);
}
static void	call_putnbr(int fd, int a) { ft_putnbr_fd(a, fd); }

static int	c_putchar(int i)
{
	return (i == 0 ? check_fd_out(call_putchar, 'a', "a")
		: check_fd_out(call_putchar, 0xe9, "\xe9"));
}

static int	c_putstr(int i)
{
	return (check_fd_out(call_putstr, !i, i ? "" : "hola mundo"));
}

static int	c_putendl(int i)
{
	return (check_fd_out(call_putendl, !i, i ? "\n" : "hola\n"));
}

static const int	g_pnbr[] = {0, 42, -42, INT_MAX, INT_MIN};

static int	c_putnbr(int i)
{
	char	e[16];

	snprintf(e, sizeof(e), "%d", g_pnbr[i]);
	return (check_fd_out(call_putnbr, g_pnbr[i], e));
}

static void	s_putchar_fd(void)
{
	run("ft_putchar_fd('a', fd)", c_putchar, 0);
	run("ft_putchar_fd('\\xe9', fd) escribe el byte tal cual", c_putchar, 1);
}

static void	s_putstr_fd(void)
{
	run("ft_putstr_fd(\"hola mundo\", fd)", c_putstr, 0);
	run("ft_putstr_fd(\"\", fd)", c_putstr, 1);
}

static void	s_putendl_fd(void)
{
	run("ft_putendl_fd(\"hola\", fd)", c_putendl, 0);
	run("ft_putendl_fd(\"\", fd)", c_putendl, 1);
}

static void	s_putnbr_fd(void)
{
	RUN_TABLE(g_pnbr, c_putnbr, "ft_putnbr_fd(%d, fd)", g_pnbr[i_]);
}

/* -------------------------------- listas --------------------------------- */

static int	g_del_calls;

static void	del_count(void *p)
{
	(void)p;
	g_del_calls++;
}

static void	*dup_upper(void *p)
{
	char	*s;

	s = strdup(p);
	for (size_t k = 0; s && s[k]; k++)
		s[k] = (char)toupper((unsigned char)s[k]);
	return (s);
}

static void	iter_upper(void *p)
{
	for (char *s = p; *s; s++)
		*s = (char)toupper((unsigned char)*s);
}

/* Lista a -> b -> c montada a mano, sin depender de las ft_lst* del alumno. */
static t_list	*mk3(char *a, char *b, char *c)
{
	static t_list	n[3];

	n[0] = (t_list){a, &n[1]};
	n[1] = (t_list){b, &n[2]};
	n[2] = (t_list){c, NULL};
	return (&n[0]);
}

static t_list	*mk3_heap(char *a, char *b, char *c)
{
	t_list	*n0;
	t_list	*n1;
	t_list	*n2;

	n0 = malloc(sizeof(t_list));
	n1 = malloc(sizeof(t_list));
	n2 = malloc(sizeof(t_list));
	*n0 = (t_list){a, n1};
	*n1 = (t_list){b, n2};
	*n2 = (t_list){c, NULL};
	return (n0);
}

static int	c_lstnew(int i)
{
	t_list	*n;
	char	x[] = "x";

	(void)i;
	n = ft_lstnew(x);
	if (!n)
		return (ko("devuelve NULL"));
	if (n->content != x)
		return (ko("content no es el puntero recibido"));
	if (n->next != NULL)
		return (ko("next no es NULL"));
	return (1);
}

static int	c_lstadd_front(int i)
{
	t_list	*lst;
	t_list	n;

	lst = i ? NULL : mk3("a", "b", "c");
	n = (t_list){"z", (t_list *)0x1};
	ft_lstadd_front(&lst, &n);
	if (lst != &n)
		return (ko("*lst no apunta al nodo nuevo"));
	if (i == 0 && (!n.next || strcmp(n.next->content, "a")))
		return (ko("el nodo nuevo no apunta al antiguo primero"));
	if (i == 1 && n.next != NULL)
		return (ko("con lista vacia, new->next deberia ser NULL"));
	return (1);
}

static int	c_lstsize(int i)
{
	unsigned int	r;

	r = ft_lstsize(i ? NULL : mk3("a", "b", "c"));
	if (r != (i ? 0u : 3u))
		return (ko("esperado %d, obtenido %u", i ? 0 : 3, r));
	return (1);
}

static int	c_lstlast(int i)
{
	t_list	*l;
	t_list	*r;

	l = i ? NULL : mk3("a", "b", "c");
	r = ft_lstlast(l);
	if (i && r)
		return (ko("con NULL deberia devolver NULL"));
	if (!i && (!r || r != l->next->next))
		return (ko("no devuelve el ultimo nodo"));
	return (1);
}

static int	c_lstadd_back(int i)
{
	t_list	*lst;
	t_list	n;

	lst = i ? NULL : mk3("a", "b", "c");
	n = (t_list){"z", NULL};
	ft_lstadd_back(&lst, &n);
	if (i && lst != &n)
		return (ko("con lista vacia, *lst deberia pasar a ser new"));
	if (!i && lst->next->next->next != &n)
		return (ko("new no queda al final"));
	return (1);
}

static int	c_lstdelone(int i)
{
	t_list	*n;

	(void)i;
	n = malloc(sizeof(t_list));
	*n = (t_list){"x", NULL};
	g_del_calls = 0;
	ft_lstdelone(n, del_count);
	if (g_del_calls != 1)
		return (ko("del se llama %d veces, esperado 1", g_del_calls));
	return (1);
}

static int	c_lstclear(int i)
{
	t_list	*lst;

	(void)i;
	lst = mk3_heap("a", "b", "c");
	g_del_calls = 0;
	ft_lstclear(&lst, del_count);
	if (g_del_calls != 3)
		return (ko("del se llama %d veces, esperado 3", g_del_calls));
	if (lst != NULL)
		return (ko("*lst no queda a NULL"));
	return (1);
}

static int	c_lstiter(int i)
{
	char	a[] = "a";
	char	b[] = "b";
	char	c[] = "c";

	(void)i;
	ft_lstiter(mk3(a, b, c), iter_upper);
	if (strcmp(a, "A") || strcmp(b, "B") || strcmp(c, "C"))
		return (ko("esperado A B C, obtenido %s %s %s", a, b, c));
	return (1);
}

static int	c_lstmap(int i)
{
	t_list	*orig;
	t_list	*r;

	(void)i;
	orig = mk3("a", "b", "c");
	r = ft_lstmap(orig, dup_upper, free);
	if (!r)
		return (ko("devuelve NULL"));
	if (r == orig)
		return (ko("devuelve la lista original, no una nueva"));
	if (!r->next || !r->next->next || r->next->next->next)
		return (ko("la lista nueva no tiene 3 nodos"));
	if (strcmp(r->content, "A") || strcmp(r->next->content, "B")
		|| strcmp(r->next->next->content, "C"))
		return (ko("contenido esperado A B C"));
	if (strcmp(orig->content, "a"))
		return (ko("ha modificado la lista original"));
	return (1);
}

static void	s_lstnew(void) { run("ft_lstnew(content)", c_lstnew, 0); }
static void	s_lstadd_front(void)
{
	run("ft_lstadd_front en lista a->b->c", c_lstadd_front, 0);
	run("ft_lstadd_front en lista vacia", c_lstadd_front, 1);
}
static void	s_lstsize(void)
{
	run("ft_lstsize(a->b->c) == 3", c_lstsize, 0);
	run("ft_lstsize(NULL) == 0", c_lstsize, 1);
}
static void	s_lstlast(void)
{
	run("ft_lstlast(a->b->c)", c_lstlast, 0);
	run("ft_lstlast(NULL)", c_lstlast, 1);
}
static void	s_lstadd_back(void)
{
	run("ft_lstadd_back en lista a->b->c", c_lstadd_back, 0);
	run("ft_lstadd_back en lista vacia", c_lstadd_back, 1);
}
static void	s_lstdelone(void)
{
	run("ft_lstdelone llama a del una vez", c_lstdelone, 0);
}
static void	s_lstclear(void)
{
	run("ft_lstclear(a->b->c): del x3 y *lst = NULL", c_lstclear, 0);
}
static void	s_lstiter(void)
{
	run("ft_lstiter(a->b->c, a mayusculas)", c_lstiter, 0);
}
static void	s_lstmap(void)
{
	run("ft_lstmap(a->b->c, copia en mayusculas, free)", c_lstmap, 0);
}

/* --------------------------------- tabla --------------------------------- */

#define ENTRY(n)	{"ft_" #n, s_ ## n}

static const struct { const char *name; void (*suite)(void); }	g_suite[] = {
	ENTRY(isalpha), ENTRY(isdigit), ENTRY(isalnum), ENTRY(isascii),
	ENTRY(isprint), ENTRY(strlen), ENTRY(memset), ENTRY(bzero), ENTRY(memcpy),
	ENTRY(memmove), ENTRY(strlcpy), ENTRY(strlcat), ENTRY(toupper),
	ENTRY(tolower), ENTRY(strchr), ENTRY(strrchr), ENTRY(strncmp),
	ENTRY(memchr), ENTRY(memcmp), ENTRY(strnstr), ENTRY(atoi), ENTRY(calloc),
	ENTRY(strdup), ENTRY(substr), ENTRY(strjoin), ENTRY(strtrim),
	ENTRY(split), ENTRY(itoa), ENTRY(strmapi), ENTRY(striteri),
	ENTRY(putchar_fd), ENTRY(putstr_fd), ENTRY(putendl_fd), ENTRY(putnbr_fd),
	ENTRY(lstnew), ENTRY(lstadd_front), ENTRY(lstsize), ENTRY(lstlast),
	ENTRY(lstadd_back), ENTRY(lstdelone), ENTRY(lstclear), ENTRY(lstiter),
	ENTRY(lstmap),
};

/* Direccion de cada ft_ (NULL si el alumno aun no la define).              */
static void	*sym(const char *name)
{
	const struct { const char *n; void *p; }	t[] = {
#define S(x)	{"ft_" #x, (void *)(uintptr_t)ft_ ## x}
		S(isalpha), S(isdigit), S(isalnum), S(isascii), S(isprint),
		S(strlen), S(memset), S(bzero), S(memcpy), S(memmove), S(strlcpy),
		S(strlcat), S(toupper), S(tolower), S(strchr), S(strrchr),
		S(strncmp), S(memchr), S(memcmp), S(strnstr), S(atoi), S(calloc),
		S(strdup), S(substr), S(strjoin), S(strtrim), S(split), S(itoa),
		S(strmapi), S(striteri), S(putchar_fd), S(putstr_fd), S(putendl_fd),
		S(putnbr_fd), S(lstnew), S(lstadd_front), S(lstsize), S(lstlast),
		S(lstadd_back), S(lstdelone), S(lstclear), S(lstiter), S(lstmap),
#undef S
	};

	for (size_t k = 0; k < sizeof(t) / sizeof(*t); k++)
		if (!strcmp(t[k].n, name))
			return (t[k].p);
	return (NULL);
}

int	main(int argc, char **argv)
{
	init_colors();
	if (argc != 2)
		return (2);
	for (size_t k = 0; k < sizeof(g_suite) / sizeof(*g_suite); k++)
	{
		if (strcmp(g_suite[k].name, argv[1]))
			continue ;
		if (!sym(argv[1]))
		{
			printf("  %s[KO]%s   el archivo no define %s (revisa el nombre de"
				" la funcion)\n", C_KO, C_0, argv[1]);
			return (1);
		}
		g_suite[k].suite();
		return (g_nko != 0);
	}
	return (2);
}

#endif
