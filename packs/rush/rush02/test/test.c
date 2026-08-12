/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para rush02                                     */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*              (o desde test/:  make)                                        */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*              (o desde test/:  make test)                                   */
/*                                                                            */
/*   A DIFERENCIA de los runners de c00-c07, rush02 no es una coleccion de    */
/*   funciones sino un PROGRAMA, asi que aqui no hay #include del fuente ni    */
/*   -Dmain=. El test es de caja negra:                                       */
/*                                                                            */
/*     a) compila TODOS los .c de repo/ex00 con -Wall -Wextra -Werror         */
/*     b) monta un directorio de trabajo con numbers.dict y un par de dicts   */
/*        derivados (uno con espacios raros, uno roto)                        */
/*     c) lanza ./rush-02 con cada juego de argumentos en un hijo con         */
/*        timeout, captura stdout+stderr y compara                            */
/*                                                                            */
/*   Comparacion: se colapsan las tandas de espacios/tabs a uno solo y se     */
/*   recorta el blanco del final, asi que "forty  two\n" y "forty two" pasan  */
/*   igual. Los saltos de linea internos SI cuentan.                          */
/*                                                                            */
/*   ------------------------------------------------------------------------ */
/*   SUPUESTOS sobre el subject (no hay subject.txt en el pack todavia).       */
/*   Si el enunciado dice otra cosa, se corrigen aqui y ya:                    */
/*                                                                            */
/*     - `./rush-02 N`         usa numbers.dict del directorio actual          */
/*     - `./rush-02 dict N`    usa el dict indicado                            */
/*     - numero invalido, negativo o argc fuera de {2,3}  ->  "Error"          */
/*     - dict que no abre o mal formado                   ->  "Dict Error"     */
/*     - las palabras van separadas por un espacio, sin comas ni guiones:      */
/*       128 -> "one hundred twenty eight"  (no "one hundred and twenty-eight")*/
/*     - el dict tolera espacios y tabuladores alrededor de los dos puntos     */
/*                                                                            */
/*   Los casos de error estan agrupados al final de g_case para poder          */
/*   tocarlos de una pasada.                                                   */
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
#define TIMEOUT 5
#define EXDIR "ex00"
#define BINNAME "rush-02"
#define DICT "numbers.dict"

/* Topes de la tanda. Esto se lanza desde el panel con `watch`, y un programa
   que se cuelga son TIMEOUT segundos por caso: sin tope, un `while (1)` deja el
   panel tres minutos en blanco. Con el primer cuelgue ya esta dicho todo. */
#define MAX_HANG 1
#define MAX_CRASH 8

typedef struct s_case
{
	const char	*args[4];
	const char	*want;
}	t_case;

/* Los argumentos van tras argv[0] y terminan en NULL. El dict, cuando se pasa,
   es el primero: `./rush-02 <dict> <n>`. */
static const t_case	g_case[] = {
/* --- numeros sueltos del dict ------------------------------------------- */
{{"0", NULL}, "zero"},
{{"5", NULL}, "five"},
{{"9", NULL}, "nine"},
{{"10", NULL}, "ten"},
{{"13", NULL}, "thirteen"},
{{"19", NULL}, "nineteen"},
{{"20", NULL}, "twenty"},
{{"70", NULL}, "seventy"},
/* --- composicion dentro de la decena ------------------------------------ */
{{"21", NULL}, "twenty one"},
{{"42", NULL}, "forty two"},
{{"99", NULL}, "ninety nine"},
/* --- centenas ------------------------------------------------------------ */
{{"100", NULL}, "one hundred"},
{{"101", NULL}, "one hundred one"},
{{"110", NULL}, "one hundred ten"},
{{"128", NULL}, "one hundred twenty eight"},
{{"300", NULL}, "three hundred"},
{{"999", NULL}, "nine hundred ninety nine"},
/* --- millares y escalas mayores ------------------------------------------ */
{{"1000", NULL}, "one thousand"},
{{"1001", NULL}, "one thousand one"},
{{"1234", NULL}, "one thousand two hundred thirty four"},
{{"20000", NULL}, "twenty thousand"},
{{"100000", NULL}, "one hundred thousand"},
{{"999999", NULL},
	"nine hundred ninety nine thousand nine hundred ninety nine"},
{{"1000000", NULL}, "one million"},
{{"1234567", NULL},
	"one million two hundred thirty four thousand five hundred sixty seven"},
{{"1000000000", NULL}, "one billion"},
/* --- dict pasado por argumento ------------------------------------------- */
{{DICT, "42", NULL}, "forty two"},
{{DICT, "0", NULL}, "zero"},
{{"espacios.dict", "42", NULL}, "forty two"},
/* --- errores de argumentos ----------------------------------------------- */
{{"abc", NULL}, "Error"},
{{"4a2", NULL}, "Error"},
{{"-1", NULL}, "Error"},
{{"", NULL}, "Error"},
{{NULL}, "Error"},
{{DICT, "42", "sobra"}, "Error"},
/* --- errores de diccionario ---------------------------------------------- */
{{"no_existe.dict", "42", NULL}, "Dict Error"},
{{"roto.dict", "42", NULL}, "Dict Error"},
};
#define N_CASE (sizeof(g_case) / sizeof(g_case[0]))

static char			g_tmp[64];
static int			g_ok;
static int			g_ko;
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

static void	dump_log(const char *path)
{
	char	*log;

	log = slurp(path);
	if (!log)
		return ;
	if (*log)
		printf("%s%s%s", C_D, log, C_0);
	free(log);
}

/* Colapsa tandas de espacios/tabs a uno y recorta el blanco del final. Los
   saltos de linea internos se respetan: si el alumno parte la salida en varias
   lineas, eso si es una diferencia. */
static void	norm(char *s)
{
	size_t	r;
	size_t	w;
	int		sp;

	r = 0;
	w = 0;
	sp = 0;
	while (s[r] == ' ' || s[r] == '\t')
		r++;
	while (s[r])
	{
		if (s[r] == ' ' || s[r] == '\t')
			sp = 1;
		else
		{
			if (sp && w)
				s[w++] = ' ';
			sp = 0;
			s[w++] = s[r];
		}
		r++;
	}
	s[w] = '\0';
	while (w > 0 && (s[w - 1] == '\n' || s[w - 1] == '\r' || s[w - 1] == ' '))
		s[--w] = '\0';
}

/* Devuelve s entrecomillado y con los no imprimibles escapados, en uno de dos
   buffers rotatorios (para poder usarlo dos veces en el mismo printf). */
static const char	*q(const char *s)
{
	static char	buf[2][256];
	static int	turn;
	char		*d;
	size_t		k;

	d = buf[turn];
	turn = (turn + 1) % 2;
	if (!s)
		return ("NULL");
	k = 0;
	d[k++] = '"';
	while (*s && k < 240)
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

/* -------------------------------------------------------------------------- */
/*                              COMPILACION                                    */
/* -------------------------------------------------------------------------- */

/* Junta los .c de repo/ex00 entrecomillados. Devuelve cuantos ha encontrado. */
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
		if (e->d_name[0] == '.' || !strstr(e->d_name, ".c"))
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

/* Devuelve 1 si compila, 0 si no. */
static int	build(const char *repo, const char *work, char *bin, size_t nbin)
{
	char	dir[PATHSZ];
	char	srcs[PATHSZ * 4];
	char	log[PATHSZ];
	char	cmd[PATHSZ * 6];
	int		n;

	xsnprintf(dir, sizeof(dir), "%s/%s", repo, EXDIR);
	n = collect_sources(dir, srcs, sizeof(srcs));
	if (n == 0)
	{
		printf("  %s[SKIP]%s sin fuentes .c en %s\n", C_SK, C_0, dir);
		return (0);
	}
	xsnprintf(bin, nbin, "%s/%s", work, BINNAME);
	xsnprintf(log, sizeof(log), "%s/cc.log", g_tmp);
	xsnprintf(cmd, sizeof(cmd),
		"cc -Wall -Wextra -Werror -o '%s'%s > '%s' 2>&1", bin, srcs, log);
	if (system(cmd) != 0)
	{
		printf("  %s[COMPILA KO]%s con -Wall -Wextra -Werror\n", C_KO, C_0);
		dump_log(log);
		return (0);
	}
	printf("  %s[OK]%s   compila %d fuente%s con -Wall -Wextra -Werror\n",
		C_OK, C_0, n, (n == 1) ? "" : "s");
	g_ok++;
	return (1);
}

/* -------------------------------------------------------------------------- */
/*                        DIRECTORIO DE TRABAJO                                */
/* -------------------------------------------------------------------------- */

/* Reescribe el dict metiendo tabs y espacios de sobra alrededor de los dos
   puntos. El subject pide tolerarlos, asi que debe dar el mismo resultado. */
static char	*spaced_dict(const char *src)
{
	char	*out;
	size_t	i;
	size_t	k;
	size_t	len;

	len = strlen(src);
	out = malloc(len * 8 + 1);
	if (!out)
		return (NULL);
	i = 0;
	k = 0;
	while (i < len)
	{
		if (src[i] == ':')
		{
			k += (size_t)sprintf(out + k, "\t  :   ");
			while (src[i + 1] == ' ' || src[i + 1] == '\t')
				i++;
		}
		else if (src[i] == '\n')
			k += (size_t)sprintf(out + k, "  \n");
		else
			out[k++] = src[i];
		i++;
	}
	out[k] = '\0';
	return (out);
}

/* Monta el directorio donde corre el binario: numbers.dict tal cual lo tiene
   el alumno, mas los dos derivados. Devuelve 1 si el dict del repo existe. */
static int	setup_work(const char *repo, const char *work)
{
	char	path[PATHSZ];
	char	*dict;
	char	*spaced;
	int		ok;

	xsnprintf(path, sizeof(path), "%s/%s/%s", repo, EXDIR, DICT);
	dict = slurp(path);
	if (!dict)
	{
		printf("  %s[KO]%s   %s no esta en repo/%s\n", C_KO, C_0, DICT, EXDIR);
		printf("     %sel subject lo pide junto al programa%s\n", C_D, C_0);
		g_ko++;
		return (0);
	}
	ok = 1;
	xsnprintf(path, sizeof(path), "%s/%s", work, DICT);
	ok = spit(path, dict) && ok;
	spaced = spaced_dict(dict);
	if (spaced)
	{
		xsnprintf(path, sizeof(path), "%s/espacios.dict", work);
		ok = spit(path, spaced) && ok;
		free(spaced);
	}
	xsnprintf(path, sizeof(path), "%s/roto.dict", work);
	ok = spit(path, "esto no es un diccionario\n42\n: sin numero\n") && ok;
	free(dict);
	if (!ok)
		fprintf(stderr, "aviso: no se pudo montar %s\n", work);
	return (ok);
}

/* -------------------------------------------------------------------------- */
/*                              EJECUCION                                      */
/* -------------------------------------------------------------------------- */

static void	label_of(const t_case *c, char *dst, size_t n)
{
	size_t	k;
	int		i;

	k = (size_t)snprintf(dst, n, "./%s", BINNAME);
	i = 0;
	while (i < 3 && c->args[i] && k < n)
	{
		k += (size_t)snprintf(dst + k, n - k, " %s", q(c->args[i]));
		i++;
	}
	if (i == 0 && k < n)
		snprintf(dst + k, n - k, " %s(sin argumentos)%s", C_D, C_0);
}

/* Lanza el binario en work/ y devuelve su salida (stdout+stderr) en *out.
   Devuelve 0 normal, 1 si murio por senal, 2 si se colgo.
   run_case traduce eso a su propio codigo: 0 OK, 1 KO, 2 senal, 3 cuelgue. */
static int	spawn(const char *work, const t_case *c, char **out)
{
	char	*av[5];
	int		fd[2];
	pid_t	pid;
	int		st;
	int		i;

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
		if (chdir(work) != 0)
			_exit(127);
		i = 0;
		av[0] = (char *)"./" BINNAME;
		while (i < 3 && c->args[i])
		{
			av[i + 1] = (char *)c->args[i];
			i++;
		}
		av[i + 1] = NULL;
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
		execv(av[0], av);
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

static int	run_case(const char *work, const t_case *c)
{
	char	label[320];
	char	want[256];
	char	*got;
	int	 	how;

	label_of(c, label, sizeof(label));
	how = spawn(work, c, &got);
	if (how != 0)
	{
		printf("  %s[CRASH]%s %s   %s%s%s\n", C_KO, C_0, label, C_KO,
			(how == 2) ? "se cuelga (timeout)" : "muere por senal", C_0);
		free(got);
		g_ko++;
		return ((how == 2) ? 3 : 2);
	}
	if (!got)
		return (0);
	norm(got);
	xsnprintf(want, sizeof(want), "%s", c->want);
	norm(want);
	if (strcmp(got, want) == 0)
	{
		printf("  %s[OK]%s   %s\n", C_OK, C_0, label);
		g_ok++;
		free(got);
		return (0);
	}
	printf("  %s[KO]%s   %s\n", C_KO, C_0, label);
	printf("     %sesperado %s%s\n", C_D, q(want), C_0);
	printf("     %sobtenido %s%s\n", C_D, q(got), C_0);
	g_ko++;
	free(got);
	return (1);
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
	char		work[PATHSZ];
	char		bin[PATHSZ];
	size_t		i;
	int			hangs;
	int			crashes;
	int			r;

	repo = NULL;
	i = 1;
	while ((int)i < argc)
		repo = argv[i++];
	if (!repo)
		repo = (access("repo", X_OK) == 0) ? "repo" : "../repo";
	/* sin buffer: el hijo escribe en este mismo stdout y si el padre acumulase
	   su salida se imprimiria toda al final, desordenada */
	setvbuf(stdout, NULL, _IONBF, 0);
	init_colors();
	strcpy(g_tmp, "/tmp/rush02test.XXXXXX");
	if (!mkdtemp(g_tmp))
		return (perror("mkdtemp"), 1);
	xsnprintf(work, sizeof(work), "%s/work", g_tmp);
	if (mkdir(work, 0755) != 0)
		return (perror("mkdir"), cleanup(), 1);
	printf("repo: %s\n\n", repo);
	printf("%s── %s/%s%s\n", C_B, EXDIR, BINNAME, C_0);
	if (!build(repo, work, bin, sizeof(bin)))
	{
		printf("\n%s1 ejercicios%s  %s1 con fallos%s  %s0 sin fuente%s\n",
			C_B, C_0, C_KO, C_0, C_SK, C_0);
		cleanup();
		return (1);
	}
	setup_work(repo, work);
	i = 0;
	hangs = 0;
	crashes = 0;
	while (i < N_CASE)
	{
		r = run_case(work, &g_case[i++]);
		hangs += (r == 3);
		crashes += (r == 2);
		if (hangs >= MAX_HANG || crashes >= MAX_CRASH)
		{
			printf("  %s[KO]%s   tanda cortada tras %d cuelgue%s y %d senal%s\n",
				C_KO, C_0, hangs, (hangs == 1) ? "" : "s",
				crashes, (crashes == 1) ? "" : "es");
			printf("     %squedan %zu casos sin probar%s\n",
				C_D, N_CASE - i, C_0);
			g_ko++;
			break ;
		}
	}
	printf("\n%s1 ejercicios%s  %s%d con fallos%s  %s0 sin fuente%s",
		C_B, C_0, C_KO, (g_ko > 0), C_0, C_SK, C_0);
	if (g_ko)
		printf("  %s(%d tests KO)%s", C_KO, g_ko, C_0);
	printf("\n");
	cleanup();
	return (g_ko != 0);
}
