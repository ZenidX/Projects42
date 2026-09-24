/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para bsq                                        */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*              (o desde test/:  make)                                        */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*              (o desde test/:  make test)                                   */
/*                                                                            */
/*   Como rush02, bsq es un PROGRAMA y no una coleccion de funciones, asi que  */
/*   el test es de caja negra: no hay #include del fuente ni -Dmain=.          */
/*                                                                            */
/*     a) copia repo/ a un temporal y lo compila alli (asi no deja .o ni       */
/*        binarios sueltos en el repo del alumno)                             */
/*     b) si hay Makefile lo usa, y comprueba de paso que no hace relink;      */
/*        si no lo hay, avisa y compila los .c a pelo para poder seguir        */
/*     c) escribe cada mapa en un directorio de trabajo y lanza ./bsq en un    */
/*        hijo con timeout, capturando stdout+stderr juntos                    */
/*                                                                            */
/*   Comparacion: EXACTA salvo los saltos de linea del final. Aqui no se puede */
/*   colapsar el blanco como en rush02, porque el espacio es un caracter de    */
/*   mapa perfectamente valido y hay un caso que lo usa como "vacio".          */
/*                                                                            */
/*   El resultado esperado de cada mapa no va escrito a mano salvo en unos     */
/*   pocos casos: lo calcula ref_solve(), un solver de referencia (DP, el      */
/*   clasico dp[i][j] = 1 + min(arriba, izquierda, diagonal)). Antes de nada,  */
/*   self_check() lo contrasta con el ejemplo del enunciado; si no cuadra, el  */
/*   roto es el test y se para en vez de suspender a nadie por su cara.        */
/*                                                                            */
/*   ------------------------------------------------------------------------ */
/*   SUPUESTOS sobre el subject (lo que no dice literalmente):                 */
/*                                                                            */
/*     - el mensaje de error es exactamente "map error"                        */
/*     - va con salto de linea al final, y da igual si sale por stdout o por   */
/*       stderr (el test los captura juntos)                                   */
/*     - un mapa sin ningun hueco sale tal cual, sin cuadrado y sin error      */
/*     - el cuerpo del mapa solo admite vacio y obstaculo: si trae ya el       */
/*       caracter "lleno", es map error (la lectura habitual en 42; el         */
/*       enunciado no lo dice con todas las letras)                            */
/*     - entre dos salidas (o dos "map error") va UNA linea en blanco          */
/*     - el binario se llama bsq y lo deja el Makefile en la raiz del repo     */
/*                                                                            */
/* ************************************************************************** */

#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#define PATHSZ 4096
#define MAXSRC 64
#define NAMESZ 64
#define DIAGSZ 200
#define TIMEOUT 5
#define BINNAME "bsq"
#define ERRMSG "map error"

/* Topes de la tanda. Esto se lanza desde el panel con `watch`, y un programa
   que se cuelga son TIMEOUT segundos por caso: sin tope, un `while (1)` deja el
   panel un minuto largo en blanco. Con el primer cuelgue ya esta dicho todo. */
#define MAX_HANG 1
#define MAX_CRASH 8

/* -------------------------------------------------------------------------- */
/*                                 ESTADO                                      */
/* -------------------------------------------------------------------------- */

static char			g_tmp[64];
static char			g_work[PATHSZ];
static int			g_ok;
static int			g_ko;
static int			g_hang;
static int			g_crash;
static int			g_cut;
static int			g_skip;
static int			g_grp;
static int			g_grp_bad;
static int			g_ko_at_grp;
static int			g_skip_at_grp;
static unsigned int	g_seed = 20260811u;

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

/* -------------------------------------------------------------------------- */
/*                              UTILIDADES                                     */
/* -------------------------------------------------------------------------- */

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

static void	*xmalloc(size_t n)
{
	void	*p;

	p = malloc(n);
	if (!p)
	{
		fprintf(stderr, "error: sin memoria\n");
		exit(1);
	}
	return (p);
}

static char	*read_all(int fd)
{
	size_t	cap;
	size_t	len;
	ssize_t	n;
	char	*buf;
	char	*tmp;

	cap = 8192;
	len = 0;
	buf = xmalloc(cap);
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

static int	spit(const char *path, const char *txt)
{
	ssize_t	n;
	size_t	len;
	int		fd;

	fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
		return (0);
	len = strlen(txt);
	n = write(fd, txt, len);
	close(fd);
	return (n == (ssize_t)len);
}

/* Escupe el diagnostico como "error: fichero:linea mensaje". El sitio va DENTRO
   del mensaje a proposito: el panel de zentest recorta el "algo.c:" del
   principio de linea (en los packs c00-c07 sobra, porque cada ejercicio es un
   solo fuente), y aqui con seis fuentes hace falta saber cual es. El "error:"
   tampoco es decorativo: es la palabra por la que filtra el panel. */
static void	print_diag(const char *kind, char *loc, char *msg)
{
	char	*p;
	size_t	n;

	p = strrchr(loc, '/');
	if (p)
		loc = p + 1;
	n = strlen(loc);
	while (n > 0 && (loc[n - 1] == ':' || loc[n - 1] == ' '))
		loc[--n] = '\0';
	p = strstr(msg, " [-W");
	if (p)
		*p = '\0';
	if (n == 0 || strcmp(loc, "make") == 0)
		printf("%s%s: %s%s\n", C_D, kind, msg, C_0);
	else
		printf("%s%s: %s %s%s\n", C_D, kind, loc, msg, C_0);
}

/* "bsq.c:(.text+0x95): undefined reference to `ft_print_map'" se queda en
   "error: bsq.c undefined reference to `ft_print_map'": el offset dentro de
   .text no le dice nada a nadie. Si delante no hay un fuente ni un objeto
   (segun el enlazador puede venir "/usr/bin/ld:"), va sin sitio. */
static void	show_link(char *line, char *msg)
{
	char	*p;
	size_t	n;

	p = strchr(line, ':');
	if (p && p < msg)
		*p = '\0';
	n = strlen(line);
	if (n < 3 || (strcmp(line + n - 2, ".c") && strcmp(line + n - 2, ".o")))
		line[0] = '\0';
	print_diag("error", line, msg);
}

/* Una linea del log de make. Pasan sus quejas propias (Makefile que ni arranca,
   regla que falta) y las del enlazador, que es el unico fallo que no se ve
   compilando fichero a fichero: cada .c puede estar perfecto y faltar aun asi
   la funcion que los une. Los errores de cc los reparte breakdown(), asi que
   aqui se ignoran para no contarlos dos veces. */
static void	show_diag(char *line)
{
	char	*msg;
	size_t	n;

	msg = strstr(line, "undefined reference to ");
	if (msg)
		return (show_link(line, msg));
	msg = strstr(line, "*** ");
	/* el "[objetivo] Error N" es el resumen de make, no cuenta nada nuevo */
	if (!msg || strstr(msg, "] Error "))
		return ;
	*msg = '\0';
	msg += 4;
	n = strlen(msg);
	while (n > 0 && (msg[n - 1] == '.' || msg[n - 1] == ' '))
		n--;
	if (n > 4 && strncmp(msg + n - 4, "Stop", 4) == 0)
		n -= 4;
	while (n > 0 && (msg[n - 1] == '.' || msg[n - 1] == ' '))
		n--;
	msg[n] = '\0';
	print_diag("error", line, msg);
}

/* Vuelca del log lo que dice make por su cuenta. */
static void	dump_diags(const char *path)
{
	char	*log;
	char	*line;
	char	*next;

	log = slurp(path);
	if (!log)
		return ;
	line = log;
	while (line && *line)
	{
		next = strchr(line, '\n');
		if (next)
			*next++ = '\0';
		show_diag(line);
		line = next;
	}
	free(log);
}

/* Quita SOLO los saltos de linea del final. Los espacios no se tocan: hay un
   mapa que usa el espacio como caracter "vacio" y ahi el blanco del final de
   linea es parte de la respuesta. */
static void	trim_nl(char *s)
{
	size_t	n;

	n = strlen(s);
	while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r'))
		s[--n] = '\0';
}

/* Devuelve s entrecomillado y con los no imprimibles escapados, en uno de dos
   buffers rotatorios (para poder usarlo dos veces en el mismo printf). El tope
   es corto a proposito: en el panel una linea de mapa larga no cabe. */
static const char	*q(const char *s)
{
	static char	buf[2][160];
	static int	turn;
	char		*d;
	size_t		k;

	d = buf[turn];
	turn = (turn + 1) % 2;
	if (!s)
		return ("NULL");
	k = 0;
	d[k++] = '"';
	while (*s && k < 76)
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

/* Copia la linea idx (0-based) de s en dst. Devuelve 0 si no hay tal linea. */
static int	line_at(const char *s, int idx, char *dst, size_t n)
{
	const char	*e;
	size_t		len;

	while (idx > 0 && s)
	{
		s = strchr(s, '\n');
		if (s)
			s++;
		idx--;
	}
	if (!s || !*s)
		return (0);
	e = strchr(s, '\n');
	if (e)
		len = (size_t)(e - s);
	else
		len = strlen(s);
	if (len >= n)
		len = n - 1;
	memcpy(dst, s, len);
	dst[len] = '\0';
	return (1);
}

/* -------------------------------------------------------------------------- */
/*                        SOLVER DE REFERENCIA                                 */
/* -------------------------------------------------------------------------- */

typedef struct s_map
{
	char	*text;
	char	**row;
	int		rows;
	int		cols;
	char	empty;
	char	obst;
	char	full;
}	t_map;

static int	count_lines(const char *s)
{
	int	n;

	n = 0;
	while (*s)
	{
		if (*s == '\n')
			n++;
		s++;
	}
	return (n);
}

/* Trocea el texto en lineas (machacando los '\n' por '\0') y devuelve cuantas.
   La ultima linea sin '\n' final tambien cuenta. */
static int	split_lines(char *text, char **row, int max)
{
	int		n;
	char	*p;

	n = 0;
	p = text;
	while (*p && n < max)
	{
		row[n++] = p;
		p = strchr(p, '\n');
		if (!p)
			break ;
		*p = '\0';
		p++;
	}
	return (n);
}

/* Lee la cabecera: los 3 ultimos caracteres son vacio/obstaculo/lleno y lo de
   delante es el numero de lineas. Devuelve 0 si no cuadra. */
static int	parse_header(const char *h, t_map *m)
{
	size_t	len;
	size_t	i;
	long	n;

	len = strlen(h);
	if (len < 4)
		return (0);
	n = 0;
	i = 0;
	while (i < len - 3)
	{
		if (h[i] < '0' || h[i] > '9')
			return (0);
		n = n * 10 + (h[i] - '0');
		if (n > 1000000)
			return (0);
		i++;
	}
	m->rows = (int)n;
	m->empty = h[len - 3];
	m->obst = h[len - 2];
	m->full = h[len - 1];
	if (m->rows <= 0)
		return (0);
	return (m->empty != m->obst && m->empty != m->full && m->obst != m->full);
}

static int	check_body(t_map *m)
{
	int	i;
	int	j;

	m->cols = (int)strlen(m->row[1]);
	if (m->cols < 1)
		return (0);
	i = 1;
	while (i <= m->rows)
	{
		if ((int)strlen(m->row[i]) != m->cols)
			return (0);
		j = 0;
		while (j < m->cols)
		{
			if (m->row[i][j] != m->empty && m->row[i][j] != m->obst)
				return (0);
			j++;
		}
		i++;
	}
	return (1);
}

/* Devuelve 0 si el mapa no es valido. Solo se usa con mapas que monta el propio
   test, asi que un 0 aqui significa que me he equivocado escribiendo un caso. */
static int	parse_map(const char *src, t_map *m)
{
	int	total;

	memset(m, 0, sizeof(*m));
	m->text = xmalloc(strlen(src) + 1);
	strcpy(m->text, src);
	total = count_lines(src) + 1;
	m->row = xmalloc(sizeof(char *) * (size_t)total);
	total = split_lines(m->text, m->row, total);
	if (total < 2 || !parse_header(m->row[0], m))
		return (free(m->text), free(m->row), 0);
	if (m->rows > total - 1 || (total - 1 > m->rows && *m->row[m->rows + 1]))
		return (free(m->text), free(m->row), 0);
	if (!check_body(m))
		return (free(m->text), free(m->row), 0);
	return (1);
}

/* Marca el mayor cuadrado. Barriendo por filas y quedandose solo con lo
   ESTRICTAMENTE mayor, la esquina inferior derecha que gana es la mas arriba y
   luego la mas a la izquierda; como el tamano es el mismo para todas las
   candidatas, eso es tambien la esquina superior izquierda que pide el subject. */
static void	fill_best(t_map *m, int *dp)
{
	int	i;
	int	j;
	int	v;
	int	br;
	int	bc;
	int	bs;

	bs = 0;
	br = 0;
	bc = 0;
	i = 0;
	while (i < m->rows)
	{
		j = 0;
		while (j < m->cols)
		{
			if (m->row[i + 1][j] == m->obst)
				v = 0;
			else if (i == 0 || j == 0)
				v = 1;
			else
			{
				v = dp[(i - 1) * m->cols + j];
				if (dp[i * m->cols + j - 1] < v)
					v = dp[i * m->cols + j - 1];
				if (dp[(i - 1) * m->cols + j - 1] < v)
					v = dp[(i - 1) * m->cols + j - 1];
				v++;
			}
			dp[i * m->cols + j] = v;
			if (v > bs)
			{
				bs = v;
				br = i;
				bc = j;
			}
			j++;
		}
		i++;
	}
	i = br - bs + 1;
	while (i <= br)
	{
		j = bc - bs + 1;
		while (j <= bc)
			m->row[i + 1][j++] = m->full;
		i++;
	}
}

static char	*join_rows(t_map *m)
{
	char	*out;
	size_t	k;
	int		i;

	out = xmalloc((size_t)m->rows * ((size_t)m->cols + 1) + 1);
	k = 0;
	i = 1;
	while (i <= m->rows)
	{
		memcpy(out + k, m->row[i], (size_t)m->cols);
		k += (size_t)m->cols;
		out[k++] = '\n';
		i++;
	}
	out[k] = '\0';
	return (out);
}

/* Resuelve el mapa y devuelve la salida esperada (malloc). NULL si el mapa no
   es valido. */
static char	*ref_solve(const char *src)
{
	t_map	m;
	int		*dp;
	char	*out;

	if (!parse_map(src, &m))
		return (NULL);
	dp = xmalloc(sizeof(int) * (size_t)m.rows * (size_t)m.cols);
	fill_best(&m, dp);
	free(dp);
	out = join_rows(&m);
	free(m.text);
	free(m.row);
	return (out);
}

/* ref_solve() con la red de seguridad puesta: si el mapa del caso no es valido
   el fallo es mio escribiendo el test, no del alumno. */
static char	*want_of(const char *label, const char *src)
{
	char	*out;

	out = ref_solve(src);
	if (!out)
	{
		fprintf(stderr, "error interno del test: el mapa del caso \"%s\" "
			"no es valido\n", label);
		exit(1);
	}
	return (out);
}

/* -------------------------------------------------------------------------- */
/*                          GENERADOR DE MAPAS                                 */
/* -------------------------------------------------------------------------- */

/* LCG propio, no rand(): quiero que dos tandas seguidas prueben exactamente los
   mismos mapas, para que un KO se pueda repetir tal cual. */
static unsigned int	rnd(void)
{
	g_seed = g_seed * 1103515245u + 12345u;
	return ((g_seed >> 16) & 0x7fffu);
}

/* Mapa de rows x cols con ~den% de obstaculos. */
static char	*gen_map(int rows, int cols, int den)
{
	char	*s;
	size_t	k;
	int		i;
	int		j;

	s = xmalloc((size_t)(rows + 1) * ((size_t)cols + 1) + 32);
	k = (size_t)sprintf(s, "%d.ox\n", rows);
	i = 0;
	while (i < rows)
	{
		j = 0;
		while (j < cols)
		{
			s[k++] = ((int)(rnd() % 100u) < den) ? 'o' : '.';
			j++;
		}
		s[k++] = '\n';
		i++;
	}
	s[k] = '\0';
	return (s);
}

/* -------------------------------------------------------------------------- */
/*                              COMPILACION                                    */
/* -------------------------------------------------------------------------- */

static int	run_sh(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
static void	report_ok(const char *label);
static void	report_ko(const char *label, const char *d1, const char *d2);

/* Deja el diagnostico en out como "linea:col mensaje". El fichero no va porque
   ya sale en la etiqueta del caso, y el [-Wflag] del final tampoco: en el panel
   solo come ancho. */
static void	fmt_diag(char *line, char *out, size_t n)
{
	char	*msg;
	char	*loc;
	char	*p;

	msg = strstr(line, " error: ");
	if (!msg)
		return ((void)snprintf(out, n, "%s", line));
	*msg = '\0';
	msg += 8;
	p = strstr(msg, " [-W");
	if (p)
		*p = '\0';
	loc = strchr(line, ':');
	if (loc)
		loc++;
	else
		loc = line;
	p = loc + strlen(loc);
	while (p > loc && p[-1] == ':')
		*--p = '\0';
	snprintf(out, n, "%s %s", loc, msg);
}

/* Compila un solo fuente y cuenta sus errores, dejando los dos primeros en
   d1/d2. */
static int	count_errors(const char *dir, const char *file, char *d1, char *d2)
{
	char	log[PATHSZ];
	char	*txt;
	char	*line;
	char	*next;
	int		errs;

	xsnprintf(log, sizeof(log), "%s/one.log", g_tmp);
	d1[0] = '\0';
	d2[0] = '\0';
	if (run_sh("cc -Wall -Wextra -Werror -c '%s/%s' -o /dev/null > '%s' 2>&1",
			dir, file, log) == 0)
		return (0);
	txt = slurp(log);
	if (!txt)
		return (0);
	errs = 0;
	line = txt;
	while (line && *line)
	{
		next = strchr(line, '\n');
		if (next)
			*next++ = '\0';
		if (strstr(line, " error: ") && ++errs < 3)
			fmt_diag(line, (errs == 1) ? d1 : d2, DIAGSZ);
		line = next;
	}
	free(txt);
	return (errs);
}

/* readdir no da ningun orden, y una lista que baila entre tandas no hay quien
   la compare de un vistazo. */
static void	sort_names(char name[][NAMESZ], int n)
{
	char	tmp[NAMESZ];
	int		i;
	int		j;

	i = 0;
	while (i < n)
	{
		j = i + 1;
		while (j < n)
		{
			if (strcmp(name[j], name[i]) < 0)
			{
				snprintf(tmp, sizeof(tmp), "%s", name[i]);
				snprintf(name[i], NAMESZ, "%s", name[j]);
				snprintf(name[j], NAMESZ, "%s", tmp);
			}
			j++;
		}
		i++;
	}
}

static int	list_sources(const char *dir, char name[][NAMESZ], int max)
{
	struct dirent	*e;
	DIR				*d;
	size_t			len;
	int				n;

	d = opendir(dir);
	if (!d)
		return (0);
	n = 0;
	while ((e = readdir(d)) && n < max)
	{
		len = strlen(e->d_name);
		if (e->d_name[0] == '.' || len < 3 || len >= NAMESZ)
			continue ;
		if (strcmp(e->d_name + len - 2, ".c") != 0)
			continue ;
		snprintf(name[n++], NAMESZ, "%s", e->d_name);
	}
	closedir(d);
	sort_names(name, n);
	return (n);
}

/* Reparto de errores por fichero. Con un solo "no compila" y seis fuentes no
   sabes por donde empezar; asi ves cual esta limpio y cual no. Devuelve
   cuantos ficheros fallan: si son cero, el que se queja es el Makefile. */
static int	breakdown(const char *dir)
{
	char	name[MAXSRC][NAMESZ];
	char	label[NAMESZ + 32];
	char	d1[DIAGSZ];
	char	d2[DIAGSZ];
	int		n;
	int		i;
	int		bad;
	int		errs;

	n = list_sources(dir, name, MAXSRC);
	bad = 0;
	i = 0;
	while (i < n)
	{
		errs = count_errors(dir, name[i], d1, d2);
		if (errs == 0)
		{
			xsnprintf(label, sizeof(label), "%-22s compila", name[i]);
			report_ok(label);
		}
		else
		{
			xsnprintf(label, sizeof(label), "%-22s %d error%s", name[i],
				errs, (errs == 1) ? "" : "es");
			report_ko(label, d1[0] ? d1 : NULL, d2[0] ? d2 : NULL);
			bad++;
		}
		i++;
	}
	return (bad);
}

/* Junta los .c de dir entrecomillados. Devuelve cuantos ha encontrado. */
static int	collect_sources(const char *dir, char *out, size_t n)
{
	struct dirent	*e;
	DIR				*d;
	size_t			len;
	size_t			need;
	int				count;

	d = opendir(dir);
	if (!d)
		return (0);
	out[0] = '\0';
	len = 0;
	count = 0;
	while ((e = readdir(d)))
	{
		if (e->d_name[0] == '.' || strlen(e->d_name) < 3)
			continue ;
		if (strcmp(e->d_name + strlen(e->d_name) - 2, ".c") != 0)
			continue ;
		need = strlen(dir) + strlen(e->d_name) + 5;
		if (len + need >= n)
			break ;
		len += (size_t)snprintf(out + len, n - len, " '%s/%s'", dir, e->d_name);
		count++;
	}
	closedir(d);
	return (count);
}

static int	run_sh(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

static int	run_sh(const char *fmt, ...)
{
	char	cmd[PATHSZ * 6];
	va_list	ap;

	va_start(ap, fmt);
	if (vsnprintf(cmd, sizeof(cmd), fmt, ap) < 0)
	{
		va_end(ap);
		return (-1);
	}
	va_end(ap);
	return (system(cmd));
}

/* Huella del binario para el test de relink: si make lo vuelve a enlazar cambia
   el inodo o la marca de tiempo (con nanosegundos, que si no un make rapido
   cabria entero dentro del mismo segundo). */
static int	stamp(const char *path, unsigned long long *out)
{
	struct stat	st;

	if (stat(path, &st) != 0)
		return (0);
	out[0] = (unsigned long long)st.st_ino;
	out[1] = (unsigned long long)st.st_mtime;
	out[2] = (unsigned long long)st.st_mtim.tv_nsec;
	return (1);
}

static void	report_ok(const char *label)
{
	printf("  %s[OK]%s   %s\n", C_OK, C_0, label);
	g_ok++;
}

static void	report_ko(const char *label, const char *d1, const char *d2)
{
	printf("  %s[KO]%s   %s\n", C_KO, C_0, label);
	if (d1)
		printf("     %s%s%s\n", C_D, d1, C_0);
	if (d2)
		printf("     %s%s%s\n", C_D, d2, C_0);
	g_ko++;
}

/* Cuenta los ficheros de dir acabados en ext. */
static int	count_ext(const char *dir, const char *ext)
{
	struct dirent	*e;
	DIR				*d;
	size_t			len;
	size_t			n;
	int				count;

	d = opendir(dir);
	if (!d)
		return (0);
	count = 0;
	n = strlen(ext);
	while ((e = readdir(d)))
	{
		len = strlen(e->d_name);
		if (len > n && strcmp(e->d_name + len - n, ext) == 0)
			count++;
	}
	closedir(d);
	return (count);
}

/* make <regla> en el directorio de trabajo. --no-print-directory quita el
   "Entering directory /tmp/..." que aqui no le dice nada a nadie. */
static int	make_rule(const char *dir, const char *rule)
{
	char	log[PATHSZ];

	xsnprintf(log, sizeof(log), "%s/mk.log", g_tmp);
	return (run_sh("make --no-print-directory -C '%s' %s > '%s' 2>&1",
			dir, rule, log));
}

/* make dos veces seguidas: la segunda no debe tocar el binario. */
static void	check_relink(const char *dir, const char *bin)
{
	unsigned long long	a[3];
	unsigned long long	b[3];

	if (!stamp(bin, a))
		return ;
	if (make_rule(dir, "") != 0)
		return (report_ko("un segundo make no falla", "make devuelve error "
				"la segunda vez", NULL));
	if (!stamp(bin, b))
		return (report_ko("un segundo make no falla",
				"el segundo make se ha cargado el binario", NULL));
	if (a[0] != b[0] || a[1] != b[1] || a[2] != b[2])
		return (report_ko("el Makefile no hace relink",
				"un segundo make vuelve a enlazar " BINNAME,
				"suele ser la regla que enlaza sin depender de los .o"));
	report_ok("el Makefile no hace relink");
}

/* clean se lleva los objetos y deja el programa. */
static void	check_clean(const char *dir, const char *bin)
{
	if (make_rule(dir, "clean") != 0)
		return (report_ko("make clean", "no hay regla clean o devuelve error",
				"clean borra los .o, los ficheros objeto"));
	if (count_ext(dir, ".o") > 0)
		return (report_ko("make clean borra los .o",
				"quedan ficheros .o despues de clean", NULL));
	if (access(bin, F_OK) != 0)
		return (report_ko("make clean deja el programa",
				"clean se ha llevado tambien " BINNAME,
				"borrar el programa es cosa de fclean, no de clean"));
	report_ok("make clean borra los .o y deja el programa");
}

/* fclean deja el directorio como estaba, y re lo reconstruye todo. */
static void	check_fclean_re(const char *dir, const char *bin)
{
	if (make_rule(dir, "fclean") != 0)
		report_ko("make fclean", "no hay regla fclean o devuelve error",
			"fclean borra los .o y ademas el programa");
	else if (access(bin, F_OK) == 0)
		report_ko("make fclean borra el programa",
			BINNAME " sigue ahi despues de fclean", NULL);
	else
		report_ok("make fclean borra los .o y el programa");
	if (make_rule(dir, "re") != 0)
		report_ko("make re", "no hay regla re o devuelve error",
			"re es un fclean seguido de un all");
	else if (access(bin, X_OK) != 0)
		report_ko("make re reconstruye el programa",
			"re no ha dejado " BINNAME, NULL);
	else
		report_ok("make re reconstruye el programa desde cero");
}

/* Compila con el Makefile del alumno y comprueba de paso que las reglas de
   siempre (all, clean, fclean, re) hacen lo que se espera de ellas.
   Devuelve 1 si al final hay binario que lanzar. */
static int	build_make(const char *dir, char *bin, size_t nbin)
{
	char	log[PATHSZ];

	xsnprintf(log, sizeof(log), "%s/mk.log", g_tmp);
	xsnprintf(bin, nbin, "%s/%s", dir, BINNAME);
	if (make_rule(dir, "") != 0)
	{
		printf("  %s[COMPILA KO]%s make no llega a construir el programa\n",
			C_KO, C_0);
		dump_diags(log);
		breakdown(dir);
		return (g_ko++, 0);
	}
	if (access(bin, X_OK) != 0)
	{
		printf("  %s[COMPILA KO]%s make no deja un ejecutable llamado %s\n",
			C_KO, C_0, BINNAME);
		printf("     %srevisa NAME en el Makefile (ojo a los espacios de "
			"mas)%s\n", C_D, C_0);
		dump_diags(log);
		return (g_ko++, 0);
	}
	report_ok("make compila y deja ./" BINNAME);
	check_relink(dir, bin);
	check_clean(dir, bin);
	check_fclean_re(dir, bin);
	if (access(bin, X_OK) != 0 && make_rule(dir, "") != 0)
		return (0);
	return (access(bin, X_OK) == 0);
}

/* Sin Makefile: cc directo a todos los .c, solo para que la tanda pueda seguir. */
static int	build_cc(const char *dir, char *bin, size_t nbin)
{
	char	srcs[PATHSZ * 4];
	char	log[PATHSZ];
	int		n;

	n = collect_sources(dir, srcs, sizeof(srcs));
	if (n == 0)
	{
		printf("  %s[SKIP]%s sin fuentes .c en repo/\n", C_SK, C_0);
		return (0);
	}
	xsnprintf(bin, nbin, "%s/%s", dir, BINNAME);
	xsnprintf(log, sizeof(log), "%s/cc.log", g_tmp);
	if (run_sh("cc -Wall -Wextra -Werror -o '%s'%s > '%s' 2>&1",
			bin, srcs, log) != 0)
	{
		printf("  %s[COMPILA KO]%s con -Wall -Wextra -Werror\n", C_KO, C_0);
		breakdown(dir);
		return (g_ko++, 0);
	}
	printf("  %s[OK]%s   compila %d fuente%s con -Wall -Wextra -Werror\n",
		C_OK, C_0, n, (n == 1) ? "" : "s");
	g_ok++;
	return (1);
}

/* Copia el repo a un temporal y lo compila alli. Devuelve 1 si hay binario en
   g_work/bsq listo para lanzar. */
static int	build(const char *repo)
{
	char	dir[PATHSZ];
	char	bin[PATHSZ];
	char	mk[PATHSZ];
	int		ok;

	xsnprintf(dir, sizeof(dir), "%s/build", g_tmp);
	if (mkdir(dir, 0755) != 0
		|| run_sh("cp -R '%s/.' '%s' 2>/dev/null", repo, dir) != 0)
	{
		printf("  %s[SKIP]%s no se puede leer %s\n", C_SK, C_0, repo);
		return (0);
	}
	xsnprintf(mk, sizeof(mk), "%s/Makefile", dir);
	if (access(mk, F_OK) == 0)
		ok = build_make(dir, bin, sizeof(bin));
	else
	{
		report_ko("el repo trae Makefile", "el subject lo pide como archivo a "
			"entregar", "mientras tanto el test compila los .c a pelo");
		ok = build_cc(dir, bin, sizeof(bin));
	}
	if (!ok)
		return (0);
	if (run_sh("cp '%s' '%s/%s'", bin, g_work, BINNAME) != 0)
	{
		printf("  %s[SKIP]%s no se puede copiar el binario al directorio de "
			"trabajo\n", C_SK, C_0);
		return (0);
	}
	return (1);
}

/* -------------------------------------------------------------------------- */
/*                              EJECUCION                                      */
/* -------------------------------------------------------------------------- */

/* Lanza ./bsq dentro de g_work y devuelve su salida (stdout+stderr) en *out.
   Devuelve 0 normal, 1 si murio por senal, 2 si se colgo. */
static int	spawn(char *const av[], const char *in_path, char **out)
{
	int		fd[2];
	pid_t	pid;
	int		st;
	int		in;

	if (pipe(fd) != 0)
		return (*out = NULL, 1);
	pid = fork();
	if (pid < 0)
		return (close(fd[0]), close(fd[1]), *out = NULL, 1);
	if (pid == 0)
	{
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		dup2(fd[1], STDERR_FILENO);
		close(fd[1]);
		if (chdir(g_work) != 0)
			_exit(127);
		in = open(in_path ? in_path : "/dev/null", O_RDONLY);
		if (in < 0)
			_exit(127);
		dup2(in, STDIN_FILENO);
		close(in);
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
		execv("./" BINNAME, av);
		_exit(127);
	}
	close(fd[1]);
	*out = read_all(fd[0]);
	close(fd[0]);
	waitpid(pid, &st, 0);
	if (WIFSIGNALED(st))
		return ((WTERMSIG(st) == SIGALRM) ? 2 : 1);
	return (0);
}

/* Localiza la primera linea distinta y la vuelca en dos lineas de detalle, que
   es justo lo que se lleva el panel de zentest detras de cada [KO]. */
static void	show_diff(const char *want, const char *got)
{
	char	lw[256];
	char	lg[256];
	int		hw;
	int		hg;
	int		n;

	n = 0;
	while (1)
	{
		hw = line_at(want, n, lw, sizeof(lw));
		hg = line_at(got, n, lg, sizeof(lg));
		if ((!hw && !hg) || !hw || !hg || strcmp(lw, lg) != 0)
			break ;
		n++;
	}
	printf("     %sesperado L%d %s%s\n", C_D, n + 1,
		hw ? q(lw) : "(nada mas)", C_0);
	printf("     %sobtenido L%d %s%s\n", C_D, n + 1,
		hg ? q(lg) : "(nada mas)", C_0);
}

/* Corta la tanda cuando el programa se cuelga o revienta demasiadas veces. */
static int	stop_now(void)
{
	return (g_hang >= MAX_HANG || g_crash >= MAX_CRASH);
}

static int	do_run(const char *label, const char *want, char *av[],
		const char *in_path)
{
	char	*got;
	char	*exp;
	int		how;

	if (stop_now())
		return (g_skip++, 4);
	how = spawn(av, in_path, &got);
	if (how != 0)
	{
		printf("  %s[CRASH]%s %s   %s%s%s\n", C_KO, C_0, label, C_KO,
			(how == 2) ? "se cuelga (timeout)" : "muere por senal", C_0);
		free(got);
		g_hang += (how == 2);
		g_crash += (how != 2);
		g_ko++;
		return (how == 2 ? 3 : 2);
	}
	if (!got)
		return (0);
	exp = xmalloc(strlen(want) + 1);
	strcpy(exp, want);
	trim_nl(got);
	trim_nl(exp);
	if (strcmp(got, exp) == 0)
		return (report_ok(label), free(got), free(exp), 0);
	printf("  %s[KO]%s   %s\n", C_KO, C_0, label);
	show_diff(exp, got);
	g_ko++;
	free(got);
	free(exp);
	return (1);
}

/* Escribe los n mapas como m1..mn y lanza ./bsq con todos como argumentos. */
static int	run_files(const char *label, const char *want,
		const char *maps[], int n)
{
	static char	name[8][8];
	char		path[PATHSZ];
	char		*av[10];
	int			i;

	i = 0;
	while (i < n)
	{
		sprintf(name[i], "m%d", i + 1);
		xsnprintf(path, sizeof(path), "%s/%s", g_work, name[i]);
		if (!spit(path, maps[i]))
			return (report_ko(label, "no se ha podido escribir el mapa", NULL), 1);
		av[i + 1] = name[i];
		i++;
	}
	av[0] = (char *)"./" BINNAME;
	av[n + 1] = NULL;
	return (do_run(label, want, av, NULL));
}

/* Un solo mapa por argumento. */
static int	run_map(const char *label, const char *map, const char *want)
{
	const char	*maps[1];

	maps[0] = map;
	return (run_files(label, want, maps, 1));
}

/* Un solo mapa, pero resuelto por el solver de referencia. */
static int	run_solved(const char *label, const char *map)
{
	char	*want;
	int		r;

	want = want_of(label, map);
	r = run_map(label, map, want);
	free(want);
	return (r);
}

/* El mapa entra por stdin y no se pasa ningun argumento. */
static int	run_stdin(const char *label, const char *map, const char *want)
{
	char	path[PATHSZ];
	char	*av[2];

	if (map)
	{
		xsnprintf(path, sizeof(path), "%s/in", g_work);
		if (!spit(path, map))
			return (report_ko(label, "no se ha podido escribir el mapa", NULL), 1);
	}
	av[0] = (char *)"./" BINNAME;
	av[1] = NULL;
	return (do_run(label, want, av, map ? "in" : "/dev/null"));
}

/* -------------------------------------------------------------------------- */
/*                                 CASOS                                       */
/* -------------------------------------------------------------------------- */

#define SUBJ_MAP "9.ox\n" \
	"...........................\n" \
	"....o......................\n" \
	"............o..............\n" \
	"...........................\n" \
	"....o......................\n" \
	"...............o...........\n" \
	"...........................\n" \
	"......o..............o.....\n" \
	"..o.......o................\n"

#define SUBJ_OUT ".....xxxxxxx...............\n" \
	"....oxxxxxxx...............\n" \
	".....xxxxxxxo..............\n" \
	".....xxxxxxx...............\n" \
	"....oxxxxxxx...............\n" \
	".....xxxxxxx...o...........\n" \
	".....xxxxxxx...............\n" \
	"......o..............o.....\n" \
	"..o.......o................\n"

typedef struct s_case
{
	const char	*label;
	const char	*map;
	const char	*want;
}	t_case;

/* Casos con la salida escrita a mano: son los que documentan las reglas finas
   (desempates, caracteres raros, mapas degenerados). El resto del grupo va con
   el solver. */
static const t_case	g_hand[] = {
{"ejemplo del enunciado (9x27)", SUBJ_MAP, SUBJ_OUT},
{"1x1 vacio", "1.ox\n.\n", "x\n"},
{"1x1 con obstaculo", "1.ox\no\n", "o\n"},
{"3x3 todo vacio", "3.ox\n...\n...\n...\n", "xxx\nxxx\nxxx\n"},
{"1 fila: el cuadrado es de 1", "1.ox\n.....\n", "x....\n"},
{"1 columna: el cuadrado es de 1", "3.ox\n.\n.\n.\n", "x\n.\n.\n"},
{"sin ningun hueco: el mapa sale tal cual",
	"2.ox\noo\noo\n", "oo\noo\n"},
{"empate: gana el de mas arriba, luego el de mas a la izquierda",
	"5.ox\n....o\n....o\nooooo\n....o\n....o\n",
	"xx..o\nxx..o\nooooo\n....o\n....o\n"},
{"empate en la misma fila: gana el de la izquierda",
	"2.ox\n..o..\n..o..\n", "xxo..\nxxo..\n"},
{"el espacio tambien es un caracter de mapa",
	"3 ox\n   \n o \n   \n", "x  \n o \n   \n"},
{"los digitos tambien valen como caracteres",
	"2.12\n..1\n...\n", "221\n22.\n"},
{"cabecera de mas de un digito",
	"10.ox\n..\n..\n..\n..\n..\n..\n..\n..\n..\n..\n",
	"xx\nxx\n..\n..\n..\n..\n..\n..\n..\n..\n"},
};
#define N_HAND (sizeof(g_hand) / sizeof(g_hand[0]))

/* kind: 0 escribe el mapa, 1 fichero que no existe, 2 un directorio */
typedef struct s_err
{
	const char	*label;
	const char	*map;
	int			kind;
}	t_err;

static const t_err	g_err[] = {
{"fichero vacio", "", 0},
{"cabecera sola, sin mapa", "9.ox\n", 0},
{"cero lineas", "0.ox\n", 0},
{"numero negativo", "-3.ox\n...\n...\n...\n", 0},
{"cabecera sin numero", ".ox\n...\n", 0},
{"cabecera sin los tres caracteres", "9\n...\n", 0},
{"vacio y obstaculo iguales", "3..x\n...\n...\n...\n", 0},
{"obstaculo y lleno iguales", "3.oo\n...\n...\n...\n", 0},
{"caracter no imprimible en la cabecera", "2.o\t\n..\n..\n", 0},
{"faltan lineas", "3.ox\n...\n...\n", 0},
{"sobran lineas", "2.ox\n...\n...\n...\n", 0},
{"lineas de distinta longitud", "2.ox\n...\n..\n", 0},
{"linea vacia dentro del mapa", "2.ox\n\n\n", 0},
{"caracter que no es ni vacio ni obstaculo", "2.ox\n..a\n...\n", 0},
{"el caracter lleno aparece en el mapa", "2.ox\n..x\n...\n", 0},
{"fichero que no existe", NULL, 1},
{"el argumento es un directorio", NULL, 2},
};
#define N_ERR (sizeof(g_err) / sizeof(g_err[0]))

/* -------------------------------------------------------------------------- */
/*                                 GRUPOS                                      */
/* -------------------------------------------------------------------------- */

static void	begin_group(const char *name)
{
	printf("%s── %s%s\n", C_B, name, C_0);
	g_ko_at_grp = g_ko;
	g_skip_at_grp = g_skip;
	g_grp++;
}

/* El motivo del corte se explica una sola vez; cada grupo dice cuantos casos
   suyos se ha comido, para que en el panel no parezca que iban bien. */
static void	end_group(void)
{
	int	skipped;

	skipped = g_skip - g_skip_at_grp;
	if (skipped > 0)
	{
		if (!g_cut)
		{
			g_cut = 1;
			printf("  %s[KO]%s   tanda cortada tras %d cuelgue%s y %d senal%s\n",
				C_KO, C_0, g_hang, (g_hang == 1) ? "" : "s",
				g_crash, (g_crash == 1) ? "" : "es");
			printf("     %sarregla primero eso y vuelve a lanzar%s\n", C_D, C_0);
			g_ko++;
		}
		printf("  %s[KO]%s   %d casos sin probar\n", C_KO, C_0, skipped);
		g_ko++;
	}
	if (g_ko > g_ko_at_grp)
		g_grp_bad++;
	printf("\n");
}

static void	group_maps(void)
{
	char	*map;
	size_t	i;

	begin_group("bsq/mapas");
	i = 0;
	while (i < N_HAND)
	{
		run_map(g_hand[i].label, g_hand[i].map, g_hand[i].want);
		i++;
	}
	map = gen_map(20, 30, 12);
	run_solved("mapa generado 20x30, 12% de obstaculos", map);
	free(map);
	map = gen_map(37, 11, 25);
	run_solved("mapa generado 37x11, 25% de obstaculos", map);
	free(map);
	map = gen_map(60, 60, 3);
	run_solved("mapa generado 60x60, 3% de obstaculos", map);
	free(map);
	map = gen_map(40, 40, 60);
	run_solved("mapa generado 40x40, 60% de obstaculos", map);
	free(map);
	map = gen_map(50, 50, 100);
	run_solved("mapa generado 50x50, todo obstaculos", map);
	free(map);
	end_group();
}

static void	group_errors(void)
{
	char	path[PATHSZ];
	char	*av[3];
	size_t	i;

	begin_group("bsq/errores");
	i = 0;
	while (i < N_ERR)
	{
		if (g_err[i].kind == 0)
			run_map(g_err[i].label, g_err[i].map, ERRMSG);
		else
		{
			av[0] = (char *)"./" BINNAME;
			av[1] = (char *)(g_err[i].kind == 1 ? "no_existe" : "subdir");
			av[2] = NULL;
			if (g_err[i].kind == 2)
			{
				xsnprintf(path, sizeof(path), "%s/subdir", g_work);
				mkdir(path, 0755);
			}
			do_run(g_err[i].label, ERRMSG, av, NULL);
		}
		i++;
	}
	end_group();
}

/* Pega salidas separadas por una linea en blanco. Cada trozo se cierra con su
   '\n' (ERRMSG no lo trae) y el separador es un '\n' de mas. */
static char	*join_out(const char *parts[], int n)
{
	char	*out;
	size_t	len;
	size_t	k;
	int		i;

	len = 0;
	i = 0;
	while (i < n)
		len += strlen(parts[i++]) + 2;
	out = xmalloc(len + 1);
	k = 0;
	i = 0;
	while (i < n)
	{
		if (i > 0)
			out[k++] = '\n';
		k += (size_t)sprintf(out + k, "%s", parts[i]);
		if (out[k - 1] != '\n')
			out[k++] = '\n';
		i++;
	}
	out[k] = '\0';
	return (out);
}

#define MAP_ROTO "2.ox\n...\n..\n"
#define MAP_3X3 "3.ox\n...\n.o.\n...\n"

static void	group_args(void)
{
	const char	*maps[3];
	const char	*parts[3];
	char		*s1;
	char		*s2;
	char		*want;

	begin_group("bsq/argumentos");
	s1 = want_of("stdin", SUBJ_MAP);
	s2 = want_of("segundo mapa", MAP_3X3);
	run_stdin("sin argumentos, el mapa entra por stdin", SUBJ_MAP, s1);
	run_stdin("sin argumentos y stdin vacio", NULL, ERRMSG);
	maps[0] = SUBJ_MAP;
	maps[1] = MAP_3X3;
	parts[0] = s1;
	parts[1] = s2;
	want = join_out(parts, 2);
	run_files("dos mapas: linea en blanco entre las dos salidas", want, maps, 2);
	free(want);
	maps[1] = MAP_ROTO;
	parts[1] = ERRMSG;
	want = join_out(parts, 2);
	run_files("mapa valido + mapa roto", want, maps, 2);
	free(want);
	maps[0] = MAP_ROTO;
	maps[1] = SUBJ_MAP;
	parts[0] = ERRMSG;
	parts[1] = s1;
	want = join_out(parts, 2);
	run_files("mapa roto + mapa valido", want, maps, 2);
	free(want);
	maps[0] = "1.ox\n.\n";
	maps[1] = MAP_ROTO;
	maps[2] = MAP_3X3;
	parts[0] = "x\n";
	parts[1] = ERRMSG;
	parts[2] = s2;
	want = join_out(parts, 3);
	run_files("tres mapas: valido, roto y valido", want, maps, 3);
	free(want);
	free(s1);
	free(s2);
	end_group();
}

static void	group_perf(void)
{
	char	*map;

	begin_group("bsq/rendimiento");
	map = gen_map(300, 300, 8);
	run_solved("mapa 300x300 en menos de 5s", map);
	free(map);
	map = gen_map(3000, 40, 10);
	run_solved("mapa 3000x40 (muchas lineas)", map);
	free(map);
	map = gen_map(6, 4000, 10);
	run_solved("mapa 6x4000 (lineas muy largas)", map);
	free(map);
	end_group();
}

/* -------------------------------------------------------------------------- */
/*                                  MAIN                                       */
/* -------------------------------------------------------------------------- */

/* El solver de referencia manda sobre casi todos los casos, asi que antes de
   nada se contrasta con el unico resultado que viene firmado por el subject. */
static void	self_check(void)
{
	char	*out;

	out = ref_solve(SUBJ_MAP);
	if (out && strcmp(out, SUBJ_OUT) == 0)
		return (free(out));
	fprintf(stderr, "error interno del test: el solver de referencia no "
		"reproduce el ejemplo del enunciado\n");
	free(out);
	exit(1);
}

static void	cleanup(void)
{
	if (g_tmp[0] && run_sh("rm -rf '%s'", g_tmp) != 0)
		fprintf(stderr, "aviso: no se pudo limpiar %s\n", g_tmp);
}

static void	summary(void)
{
	printf("%s%d ejercicios%s  %s%d con fallos%s  %s0 sin fuente%s",
		C_B, g_grp, C_0, C_KO, g_grp_bad, C_0, C_SK, C_0);
	if (g_ko)
		printf("  %s(%d tests KO)%s", C_KO, g_ko, C_0);
	printf("\n");
}

int	main(int argc, char **argv)
{
	const char	*repo;
	int			i;

	repo = NULL;
	i = 1;
	while (i < argc)
		repo = argv[i++];
	if (!repo)
		repo = (access("repo", X_OK) == 0) ? "repo" : "../repo";
	/* sin buffer: el hijo escribe en este mismo stdout y si el padre acumulase
	   su salida se imprimiria toda al final, desordenada */
	setvbuf(stdout, NULL, _IONBF, 0);
	init_colors();
	self_check();
	strcpy(g_tmp, "/tmp/bsqtest.XXXXXX");
	if (!mkdtemp(g_tmp))
		return (perror("mkdtemp"), 1);
	xsnprintf(g_work, sizeof(g_work), "%s/work", g_tmp);
	if (mkdir(g_work, 0755) != 0)
		return (perror("mkdir"), cleanup(), 1);
	printf("repo: %s\n\n", repo);
	begin_group("bsq/compila");
	if (!build(repo))
		return (end_group(), summary(), cleanup(), 1);
	end_group();
	group_maps();
	group_errors();
	group_args();
	group_perf();
	summary();
	cleanup();
	return (g_ko != 0);
}
