/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para ft_printf                                  */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*              (o desde test/:  make)                                        */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*              (o desde test/:  make test)                                   */
/*                                                                            */
/*   Este archivo hace dos papeles:                                           */
/*                                                                            */
/*   a) RUNNER (compilacion normal):                                          */
/*      - que esten Makefile, ft_printf.h y algun .c, que todos los .c y .h  */
/*        (libft/ incluida) pasen norminette y que ft_printf.h declare       */
/*        int ft_printf(const char *, ...);                                   */
/*      - el Makefile, probado en una copia del repo para no dejar .o ni .a: */
/*        flags, reglas, sin libtool, libftprintf.a, libft con su Makefile,  */
/*        sin relink, clean, fclean, re y, si la hay, la regla bonus;        */
/*      - con nm sobre libftprintf.a: que defina ft_printf, que no llame a   */
/*        nada fuera de malloc, free y write y que no tape a la libc;        */
/*      - se recompila a si mismo con -DFT_CASES enlazando la libftprintf.a  */
/*        de make (y la de make bonus) y lanza los casos. Si el Makefile no  */
/*        la crea, el runner compila los .c (sin los _bonus.c) por su cuenta */
/*        para que los casos se puedan probar igual.                         */
/*                                                                            */
/*   b) CASOS (-DFT_CASES). Cada caso corre en un hijo con timeout y compara */
/*      lo que ft_printf escribe en el fd 1 y lo que devuelve con lo que da  */
/*      snprintf de glibc con el mismo formato y argumentos: la referencia   */
/*      es la de Linux, asi que %p de NULL es "(nil)" y %s de NULL "(null)". */
/*      malloc/free estan vigilados como en Libft: un caso que pasa no puede */
/*      dejar memoria viva, y si ft_printf usa malloc se repite una llamada  */
/*      haciendo fallar el malloc numero k: no puede reventar ni perder      */
/*      memoria. No se prueba nada que en printf sea comportamiento          */
/*      indefinido (un '%' suelto al final, %#d, precision en %c o %p...).  */
/*                                                                            */
/*   Anadir un caso = una linea P(...) en la funcion c_* de su grupo.         */
/*   Anadir un grupo = una funcion c_* y su each() en suite_mand() o          */
/*   suite_bonus().                                                           */
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
#define LIB		"libftprintf.a"

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

static int		g_has_norm;
static char		*g_libc;
static char		*g_inc;
static char		g_tmp[] = "/tmp/printf_runner.XXXXXX";

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

/* Nombres que exporta la libc, rodeados por ' ' para buscar " x " con      */
/* strstr. Una funcion auxiliar que se llame strlen o putchar a secas tapa  */
/* a la de la libc en todo el programa que enlace libftprintf.a.             */
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

/* Los -I de todos los directorios del repo con algun .h, para el probe de  */
/* la cabecera y para compilar los .c si el Makefile no lo hace.            */
static void	load_inc(const char *repo)
{
	sh(&g_inc, "find '%s' -name .git -prune -o -type f -name '*.h' -print"
		" | sed 's|/[^/]*$||' | sort -u | sed \"s|.*| -I'&'|\" | tr -d '\\n'",
		repo);
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

/* --------------------------- seccion ft_printf --------------------------- */

/* Se compila un .c que toma la direccion de ft_printf con el tipo exacto:  */
/* si el prototipo es otro, -Werror lo tumba.                               */
static int	check_header(const char *repo)
{
	char	*out;
	char	*hdr;
	int		bad;

	out = NULL;
	hdr = NULL;
	sh(&hdr, "cd '%s' && find . -name .git -prune -o -type f -name ft_printf.h"
		" -print | head -n 1", repo);
	if (!*hdr)
		return (free(hdr), line(0, NULL, "ft_printf.h declara ft_printf"));
	sh(&out, "printf '#include \"ft_printf.h\"\\nint (*g_p)(const char *, ...)"
		" = ft_printf;\\n' > '%s/probe.c' && cc " CFLAGS " -fsyntax-only%s"
		" '%s/probe.c' 2>&1", g_tmp, g_inc, g_tmp);
	bad = line(*out == '\0', out, "ft_printf.h declara int ft_printf(const"
			" char *, ...)");
	free(out);
	free(hdr);
	return (bad);
}

/* Ficheros, norma y cabecera. 1 si algo falla.                             */
static int	check_static(const char *repo, const char *srcs)
{
	char	path[4096];
	char	*files;
	char	*miss;
	char	*log;
	char	*err;
	char	*ln;
	char	*sv;
	size_t	n_err;
	int		n;
	int		bad;

	miss = NULL;
	if (!exists(repo, "Makefile"))
		add_name(&miss, "Makefile");
	if (!*srcs)
		add_name(&miss, "ningun .c");
	bad = line(!miss, NULL, "hay Makefile, ft_printf.h y los .c");
	if (miss)
		printf("         falta %s\n", miss);
	free(miss);
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
		bad |= line(!log, log, "norminette (%d ficheros)", n);
		free(log);
		free(files);
	}
	bad |= check_header(repo);
	if (exists(repo, "libft"))
		bad |= line(exists(repo, "libft/Makefile"), NULL,
				"libft/ trae su Makefile");
	return (bad);
}

/* ---------------------------- seccion Makefile --------------------------- */

static int	has_rule(const char *d, const char *rule)
{
	char	*out;
	int		st;

	out = NULL;
	st = sh(&out, "make -n -C '%s' '%s' >/dev/null 2>&1", d, rule);
	free(out);
	return (st == 0);
}

/* make <rule> dos veces: la segunda no puede tocar ningun .o ni .a.       */
static int	no_relink(const char *d, const char *rule)
{
	char	*out;
	int		bad;

	out = NULL;
	sh(&out, "make -C '%s' %s >/dev/null 2>&1; touch '%s/.stamp' && make -C '%s'"
		" %s >/dev/null 2>&1; find '%s' -newer '%s/.stamp' \\( -name '*.o' -o"
		" -name '*.a' \\) | sed 's|^%s/||'", d, rule, d, d, rule, d, d, d);
	bad = line(!*out, out, "make%s%s otra vez no recompila nada (sin relink)",
			*rule ? " " : "", rule);
	free(out);
	return (bad);
}

/* El Makefile se prueba en una copia del repo (sin .git, .o ni .a). Deja   */
/* en g_tmp/mand.a la libftprintf.a de make y en g_tmp/bonus.a la de make   */
/* bonus, si las crea. *bonus_rule dice si hay regla bonus.                  */
static int	check_makefile(const char *repo, int *bonus_rule)
{
	static const char	*rules[] = {LIB, "all", "clean", "fclean", "re"};
	char				d[4200];
	char				*out;
	char				*miss;
	int					bad;
	int					st;

	*bonus_rule = 0;
	out = NULL;
	snprintf(d, sizeof(d), "%s/mk", g_tmp);
	if (!exists(repo, "Makefile"))
		return (line(0, NULL, "no existe"));
	sh(&out, "mkdir -p '%s' && tar -C '%s' --exclude=.git --exclude='*.o'"
		" --exclude='*.a' -cf - . | tar -C '%s' -xf - 2>&1", d, repo, d);
	bad = line(sh(&out, "grep -q -- -Wall '%s/Makefile' && grep -q -- -Wextra"
				" '%s/Makefile' && grep -q -- -Werror '%s/Makefile'", d, d, d) == 0,
			NULL, "usa -Wall -Wextra -Werror");
	miss = NULL;
	for (size_t k = 0; k < sizeof(rules) / sizeof(*rules); k++)
		if (!has_rule(d, rules[k]))
			add_name(&miss, rules[k]);
	bad |= line(!miss, NULL, "tiene las reglas " LIB ", all, clean, fclean y"
			" re");
	if (miss)
		printf("         faltan: %s\n", miss);
	free(miss);
	bad |= line(sh(&out, "grep -qw libtool '%s/Makefile'", d) != 0, NULL,
			"no usa libtool (el subject pide ar)");
	st = sh(&out, "make -C '%s' 2>&1 && test -f '%s/" LIB "'", d, d);
	if (line(st == 0, out, "make crea " LIB))
		return (free(out), 1);
	sh(&out, "cp '%s/" LIB "' '%s/mand.a'", d, g_tmp);
	if (exists(d, "libft"))
		bad |= line(sh(&out, "find '%s/libft' -name '*.o' | grep -q .", d) == 0,
				NULL, "make compila libft con su Makefile");
	bad |= no_relink(d, "");
	st = sh(&out, "make -C '%s' clean >/dev/null 2>&1; find '%s' -name '*.o' |"
			" sed 's|^%s/||'; test -f '%s/" LIB "'", d, d, d, d);
	bad |= line(!*out && st == 0, *out ? out : "ha borrado " LIB,
			"make clean borra los .o y deja " LIB);
	sh(&out, "make -C '%s' fclean >/dev/null 2>&1; find '%s' \\( -name '*.o'"
		" -o -name '*.a' \\) | sed 's|^%s/||'", d, d, d);
	bad |= line(!*out, out, "make fclean no deja ningun .o ni .a");
	st = sh(&out, "make -C '%s' re >/dev/null 2>&1 && test -f '%s/" LIB "'", d,
			d);
	bad |= line(st == 0, NULL, "make re vuelve a crear " LIB);
	if ((*bonus_rule = has_rule(d, "bonus")))
	{
		sh(&out, "make -C '%s' fclean >/dev/null 2>&1", d);
		st = sh(&out, "make -C '%s' bonus 2>&1 && test -f '%s/" LIB "'", d, d);
		if (!line(st == 0, out, "make bonus crea " LIB))
		{
			sh(&out, "cp '%s/" LIB "' '%s/bonus.a'", d, g_tmp);
			bad |= no_relink(d, "bonus");
		}
		else
			bad = 1;
	}
	sh(&out, "make -C '%s' fclean >/dev/null 2>&1", d);
	free(out);
	return (bad);
}

/* Sin Makefile que funcione, los .c del repo (sin los _bonus.c) a mano.   */
static int	build_own(const char *repo, const char *lib, char **log)
{
	return (sh(log, "set -e; mkdir -p '%s/own'; cd '%s'; n=0; for f in $(find ."
			" -name .git -prune -o -type f -name '*.c' ! -name '*_bonus.c'"
			" -print); do n=$((n + 1)); cc " CFLAGS "%s -c \"$f\" -o"
			" \"%s/own/$n.o\" 2>&1; done; ar rcs '%s' '%s'/own/*.o 2>&1", g_tmp,
			repo, g_inc, g_tmp, lib, g_tmp));
}

/* ------------------------------ nm ---------------------------------------- */

typedef struct s_syms
{
	char	*def;		/* " a b c ": funciones definidas (T/W) */
	char	*und;		/* " a b c ": llamadas a fuera (U) */
}	t_syms;

static void	add_sym(char **dst, const char *name)
{
	size_t	len;

	len = *dst ? strlen(*dst) : 0;
	*dst = realloc(*dst, len + strlen(name) + 3);
	if (len == 0)
		strcpy(*dst, " ");
	strcat(strcat(*dst, name), " ");
}

/* nm de un .a: cabeceras "x.o:" y lineas en blanco se saltan solas.       */
static void	scan_lib(const char *lib, t_syms *s)
{
	char	*out;
	char	*ln;
	char	t1[512];
	char	t2[512];
	char	name[512];
	int		n;

	out = NULL;
	sh(&out, "nm '%s' 2>/dev/null", lib);
	for (ln = strtok(out, "\n"); ln; ln = strtok(NULL, "\n"))
	{
		/* "addr T nombre", o "U nombre" sin direccion en los indefinidos.  */
		n = sscanf(ln, "%511s %511s %511s", t1, t2, name);
		if (n == 2 && !strcmp(t1, "U"))
			add_sym(&s->und, t2);
		else if (n == 3 && (!strcmp(t2, "T") || !strcmp(t2, "W")))
			add_sym(&s->def, name);
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

static int	check_syms(const char *repo, const char *lib, const char *tag)
{
	static const char	*ok_ext = " malloc free write __stack_chk_fail"
		" _GLOBAL_OFFSET_TABLE_ ";
	char				*forbid;
	char				*clash;
	char				*w;
	t_syms				s;
	int					bad;

	memset(&s, 0, sizeof(s));
	scan_lib(lib, &s);
	bad = line(has(s.def, "ft_printf"), NULL, "%sdefine ft_printf", tag);
	if (!has(s.def, "ft_printf"))
		printf("         (revisa el nombre y que no sea static)\n");
	forbid = NULL;
	for (w = s.und ? strtok(s.und, " ") : NULL; w; w = strtok(NULL, " "))
	{
		if (has(ok_ext, w) || has(s.def, w))
			continue ;
		if ((!strcmp(w, "memset") || !strcmp(w, "memcpy")
				|| !strcmp(w, "memmove")) && !in_sources(repo, w))
			continue ;
		add_name(&forbid, w);
	}
	bad |= line(!forbid, NULL, "%ssolo llama a malloc, free y write", tag);
	if (forbid)
		printf("         usa %s\n", forbid);
	clash = NULL;
	for (w = s.def ? strtok(s.def, " ") : NULL; w; w = strtok(NULL, " "))
		if (has(g_libc, w))
			add_name(&clash, w);
	bad |= line(!clash, NULL, "%sno redefine funciones de la libc", tag);
	if (clash)
		printf("         define %s, que tapa a la de la libc: ponle otro"
			" nombre (ft_...)\n", clash);
	free(forbid);
	free(clash);
	free(s.def);
	free(s.und);
	return (bad);
}

/* ------------------------------ casos ------------------------------------- */

static void	self_path(char *buf, size_t sz, const char *argv0)
{
	const char	*slash;

	slash = strrchr(argv0, '/');
	if (!slash)
		snprintf(buf, sz, "test.c");
	else
		snprintf(buf, sz, "%.*s/test.c", (int)(slash - argv0), argv0);
}

/* Monta el binario de casos con lib y lo lanza. -no-pie porque los .o del  */
/* alumno pueden venir sin -fPIE (clang no lo pone solo). 1 si algo falla. */
static int	run_cases(const char *self, const char *lib, const char *mode)
{
	char	bin[4200];
	char	data[4200];
	char	*log;
	pid_t	pid;
	int		st;

	snprintf(bin, sizeof(bin), "%s/cases_%s", g_tmp, mode);
	snprintf(data, sizeof(data), "%s/data", g_tmp);
	log = NULL;
	if (sh(&log, "cc -Wall -Wextra -Wno-format -no-pie -DFT_CASES '%s' '%s'"
			" -o '%s' 2>&1", self, lib, bin) != 0)
	{
		printf("  %s[TEST KO]%s no se pudo montar el binario de casos con "
			LIB "\n", C_KO, C_0);
		print_indented(log);
		free(log);
		return (1);
	}
	free(log);
	fflush(stdout);
	pid = fork();
	if (pid == 0)
	{
		execl(bin, bin, mode, data, (char *)NULL);
		_exit(127);
	}
	waitpid(pid, &st, 0);
	return (!WIFEXITED(st) || WEXITSTATUS(st) != 0);
}

int	main(int argc, char **argv)
{
	const char	*repo;
	char		self[4096];
	char		mand[4200];
	char		bonus[4200];
	char		*srcs;
	char		*log;
	int			sections;
	int			ko;
	int			skipped;
	int			bonus_rule;

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
	load_inc(repo);
	snprintf(mand, sizeof(mand), "%s/mand.a", g_tmp);
	snprintf(bonus, sizeof(bonus), "%s/bonus.a", g_tmp);
	srcs = list_files(repo, "*.c");
	sections = 1;
	ko = 0;
	skipped = 0;
	printf("%s── ft_printf%s\n", C_B, C_0);
	if (!*srcs && !exists(repo, "Makefile"))
	{
		printf("  %s[SKIP]%s no hay Makefile ni ningun .c\n\n", C_SK, C_0);
		skipped++;
	}
	else
	{
		ko += check_static(repo, srcs);
		printf("\n%s── Makefile%s\n", C_B, C_0);
		ko += check_makefile(repo, &bonus_rule);
		sections += 4;
		printf("\n%s── " LIB "%s\n", C_B, C_0);
		log = NULL;
		if (access(mand, F_OK) != 0 && !*srcs)
			printf("  %s[COMPILA KO]%s no hay " LIB " ni .c con que crearla\n",
				C_KO, C_0);
		else if (access(mand, F_OK) != 0 && build_own(repo, mand, &log))
		{
			printf("  %s[COMPILA KO]%s el Makefile no crea " LIB " y los .c no"
				" compilan con " CFLAGS "\n", C_KO, C_0);
			print_indented(log);
			unlink(mand);
		}
		else if (log)
			printf("  el Makefile no crea " LIB ": los casos usan una que"
				" compila el runner con los .c (sin los _bonus.c)\n");
		ko += access(mand, F_OK) != 0;
		if (access(mand, F_OK) == 0)
		{
			ko += check_syms(repo, mand, "");
			if (access(bonus, F_OK) == 0 && sh(&log, "cmp -s '%s' '%s'", mand,
					bonus) != 0)
				ko += check_syms(repo, bonus, "bonus: ");
		}
		printf("\n%s── casos%s\n", C_B, C_0);
		if (access(mand, F_OK) == 0)
			ko += run_cases(self, mand, "mand");
		else
			printf("  %s[TEST KO]%s sin " LIB " no hay casos que lanzar\n",
				C_KO, C_0);
		printf("\n%s── bonus%s\n", C_B, C_0);
		if (access(bonus, F_OK) == 0)
		{
			if (access(mand, F_OK) != 0 || sh(&log, "cmp -s '%s' '%s'", mand,
					bonus) != 0)
				ko += run_cases(self, bonus, "bonusmand");
			ko += run_cases(self, bonus, "bonus");
		}
		else
		{
			if (!bonus_rule)
			{
				printf("  %s[SKIP]%s el Makefile no tiene regla bonus\n",
					C_SK, C_0);
				skipped++;
			}
			else
			{
				printf("  %s[TEST KO]%s make bonus no crea " LIB "\n", C_KO,
					C_0);
				ko++;
			}
			/* Se ven igual, con la de make, pero no cuentan como fallo.     */
			if (access(mand, F_OK) == 0)
			{
				printf("  (los casos de abajo usan la " LIB " de make y no"
					" cuentan como fallo)\n");
				run_cases(self, mand, "bonus");
			}
		}
		free(log);
		printf("\n");
	}
	printf("%s%d ejercicios%s  %s%d con fallos%s  %s%d sin fuente%s\n",
		C_B, sections, C_0, C_KO, ko, C_0, C_SK, skipped, C_0);
	free(srcs);
	free(g_libc);
	free(g_inc);
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

int	ft_printf(const char *fmt, ...);

/* ------------------------------ andamiaje -------------------------------- */
/* run() lanza un hijo con alarm(). El hijo manda el detalle del fallo       */
/* (esperado/obtenido) por una tuberia y sale con 0 si pasa. Los casos        */
/* devuelven 1 (OK) o lo que devuelva ko() (0).                               */
/* each() saca cada P() de un grupo en su propio hijo: con g_want = k solo se */
/* ejecuta el P numero k, que manda antes su llamada como primera linea. Si   */
/* el grupo no llega a k (g_hit a 0) el hijo sale con 3 y se acaba el grupo.  */

typedef int	(*t_case)(int);

static int	g_fd = -1;
static int	g_nko;
static int	g_nok;
static int	g_want = -1;	/* P() que toca del grupo; -1 = todos seguidos */
static int	g_k;
static int	g_hit;
static int	g_quiet;		/* solo los fallos, con su grupo delante */
static FILE	*g_o;			/* salida del grupo en curso, en memoria */

/* Un grupo por conversion: sale con su recuento delante y en el resumen.  */
typedef struct s_group
{
	const char	*name;
	const char	*desc;
	int			ok;
	int			n;
}	t_group;

static t_group	g_groups[32];
static int		g_ngroups;
static char		*g_mem;
static size_t	g_memsz;

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
/* NULL.                                                                      */

void	*__libc_malloc(size_t n);
void	*__libc_realloc(void *p, size_t n);
void	__libc_free(void *p);

#define G_MAGIC		0x5a454e4755415244ULL
#define G_TAIL		8
#define G_JUNK		0xbe

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
	memset(h + 1, G_JUNK, n);
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

/* 0 OK, 1 KO o CRASH, 3 el grupo no tiene el caso want.                   */
static int	run(const char *name, t_case f, int i, int want)
{
	struct rlimit	rl;
	int				fds[2];
	pid_t			pid;
	int				st;
	char			buf[4096];
	ssize_t			r;
	size_t			len;
	char			*ln;
	char			*nl;
	const char		*ind;

	fflush(stdout);
	if (pipe(fds) != 0)
		return (1);
	pid = fork();
	if (pid == 0)
	{
		close(fds[0]);
		g_fd = fds[1];
		quiet_stderr();
		/* Un ft_printf que no para de escribir muere con SIGXFSZ en 16 MB. */
		rl.rlim_cur = 16 << 20;
		rl.rlim_max = 16 << 20;
		setrlimit(RLIMIT_FSIZE, &rl);
		alarm(3);
		g_want = want;
		st = check_leaks(f, i);
		if (want >= 0 && !g_hit)
			_exit(3);
		_exit(st ? 0 : 1);
	}
	close(fds[1]);
	len = 0;
	while ((r = read(fds[0], buf + len, sizeof(buf) - 1 - len)) > 0)
		len += (size_t)r;
	buf[len] = '\0';
	close(fds[0]);
	waitpid(pid, &st, 0);
	if (WIFEXITED(st) && WEXITSTATUS(st) == 3)
		return (3);
	ln = buf;
	if (want >= 0)
	{
		name = buf;
		nl = strchr(buf, '\n');
		ln = nl ? nl + 1 : buf + len;
		if (nl)
			*nl = '\0';
	}
	ind = "    ";
	if (WIFEXITED(st) && WEXITSTATUS(st) == 0)
	{
		g_nok++;
		if (!g_quiet)
			fprintf(g_o, "%s%s[OK]%s   %s\n", ind, C_OK, C_0, name);
		return (0);
	}
	g_nko++;
	if (WIFSIGNALED(st))
		fprintf(g_o, "%s%s[CRASH]%s %s (%s)\n", ind, C_KO, C_0, name,
			WTERMSIG(st) == SIGALRM ? "se cuelga, timeout 3s"
			: WTERMSIG(st) == SIGXFSZ ? "escribe sin parar"
			: strsignal(WTERMSIG(st)));
	else
	{
		fprintf(g_o, "%s%s[KO]%s   %s\n", ind, C_KO, C_0, name);
		for (ln = strtok(ln, "\n"); ln; ln = strtok(NULL, "\n"))
			fprintf(g_o, "%s       %s\n", ind, ln);
	}
	return (1);
}

/* Los casos de un grupo se escriben en memoria y group_end() los saca     */
/* detras de la cabecera, que lleva el recuento. En modo g_quiet un grupo    */
/* sin fallos no sale.                                                       */
static void	group_begin(const char *name, const char *desc)
{
	g_groups[g_ngroups].name = name;
	g_groups[g_ngroups].desc = desc;
	g_groups[g_ngroups].ok = g_nok;
	g_groups[g_ngroups].n = g_nok + g_nko;
	g_o = open_memstream(&g_mem, &g_memsz);
	if (!g_o)
		g_o = stdout;
}

static void	group_end(void)
{
	t_group	*g;

	g = &g_groups[g_ngroups++];
	g->ok = g_nok - g->ok;
	g->n = g_nok + g_nko - g->n;
	if (g_o != stdout)
		fclose(g_o);
	g_o = stdout;
	if (!g_quiet || g->ok != g->n)
		printf("  %s%-10s%s %-40s %s%d/%d%s\n", C_B, g->name, C_0, g->desc,
			g->ok == g->n ? C_OK : C_KO, g->ok, g->n, C_0);
	if (g_mem)
		fputs(g_mem, stdout);
	free(g_mem);
	g_mem = NULL;
}

/* Un grupo: cada P() suelto, hasta que el grupo se acaba.                 */
static void	each(const char *name, const char *desc, t_case f)
{
	group_begin(name, desc);
	for (int k = 0; run(NULL, f, 0, k) != 3; k++)
		;
	group_end();
}

/* Una fila por grupo: lo que se ve primero al acabar.                      */
static void	summary(const char *title)
{
	int	ok;
	int	n;

	ok = 0;
	n = 0;
	printf("\n%s── %s%s\n", C_B, title, C_0);
	for (int k = 0; k < g_ngroups; k++)
	{
		printf("  %-10s %s%4d/%-4d%s %s\n", g_groups[k].name,
			g_groups[k].ok == g_groups[k].n ? C_OK : C_KO, g_groups[k].ok,
			g_groups[k].n, C_0, g_groups[k].ok == g_groups[k].n ? "OK" : "KO");
		ok += g_groups[k].ok;
		n += g_groups[k].n;
	}
	printf("  %-10s %s%4d/%-4d%s\n", "total", ok == n ? C_OK : C_KO, ok, n,
		C_0);
}

/* Representacion legible y recortada de n bytes: "abc\n...(20001 bytes)". */
static const char	*esc(const char *s, size_t n)
{
	static char			bufs[4][512];
	static int			k;
	char				*b;
	size_t				i;
	size_t				o;

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

/* ------------------------------ capturar ---------------------------------- */
/* ft_printf escribe en el fd 1: durante la llamada el fd 1 es un fichero    */
/* temporal (g_cap, ya borrado) y luego se vuelve a g_out. Lo esperado sale  */
/* de snprintf con los mismos argumentos.                                     */

#define G_BUF	(1 << 17)

static int		g_cap = -1;
static int		g_out = -1;
static char		g_got[G_BUF];
static size_t	g_glen;
static char		g_exp[G_BUF];
static char		*g_long;

static void	cap_begin(void)
{
	if (ftruncate(g_cap, 0) != 0 || lseek(g_cap, 0, SEEK_SET) != 0)
		_exit(ko("no se pudo vaciar el fichero de captura") ? 0 : 1);
	dup2(g_cap, STDOUT_FILENO);
}

static void	cap_end(void)
{
	off_t	end;
	ssize_t	r;
	size_t	len;

	dup2(g_out, STDOUT_FILENO);
	end = lseek(g_cap, 0, SEEK_END);
	g_glen = end > 0 ? (size_t)end : 0;
	lseek(g_cap, 0, SEEK_SET);
	len = 0;
	while (len < sizeof(g_got) && (r = read(g_cap, g_got + len,
				sizeof(g_got) - len)) > 0)
		len += (size_t)r;
}

/* Compara salida y retorno de ft_printf (r) con los de snprintf (e).       */
static int	same(const char *call, int r, int e)
{
	size_t	el;
	size_t	gl;
	size_t	d;

	el = e > 0 ? (size_t)e : 0;
	gl = g_glen < sizeof(g_got) ? g_glen : sizeof(g_got);
	if (g_glen == el && !memcmp(g_got, g_exp, el))
	{
		if (r == e)
			return (1);
		return (ko("%s%s%sescribe bien %s\npero devuelve %d y esperaba %d",
				g_want < 0 ? "ft_printf(" : "", g_want < 0 ? call : "",
				g_want < 0 ? "): " : "", esc(g_exp, el), r, e));
	}
	for (d = 0; d < gl && d < el && g_got[d] == g_exp[d]; d++)
		;
	return (ko("%s%s%sesperado %s (devuelve %d)\nobtenido %s"
			" (devuelve %d, difiere en el byte %zu)",
			g_want < 0 ? "ft_printf(" : "", g_want < 0 ? call : "",
			g_want < 0 ? "): " : "", esc(g_exp, el), e, esc(g_got, g_glen), r,
			d));
}

/* Manda la llamada del caso al padre, antes de ejecutarla: si revienta,    */
/* el padre sabe igual cual era.                                            */
static void	say(const char *call)
{
	char	buf[1024];
	int		n;

	n = snprintf(buf, sizeof(buf), "ft_printf(%s)\n", call);
	if (n > (int)sizeof(buf) - 1)
		n = sizeof(buf) - 1;
	if (write(g_fd, buf, (size_t)n) < 0)
		return ;
}

/* Un caso: ft_printf y snprintf con los mismos argumentos. Con g_want = -1 */
/* corren todos y el primer KO sale de la funcion c_*; con g_want = k solo  */
/* corre el numero k y la funcion acaba ahi.                                */
#define P(...)	do { \
	int	r_; \
	int	e_; \
	if (g_want >= 0 && g_k++ != g_want) \
		break ; \
	g_hit = 1; \
	if (g_want >= 0) \
		say(#__VA_ARGS__); \
	cap_begin(); \
	r_ = ft_printf(__VA_ARGS__); \
	cap_end(); \
	e_ = snprintf(g_exp, sizeof(g_exp), __VA_ARGS__); \
	if (!same(#__VA_ARGS__, r_, e_)) \
		return (0); \
	if (g_want >= 0) \
		return (1); \
} while (0)

/* --------------------------- casos obligatorios --------------------------- */

static int	c_text(int i)
{
	(void)i;
	P("");
	P("hola");
	P("hola mundo\n");
	P("\t\n\001\177 fin");
	P("\xc3\xb1" "and\xc3\xba");
	P("%s", "%d%s%c sin convertir");
	return (1);
}

static int	c_c(int i)
{
	(void)i;
	P("%c", 'a');
	P("%c", '0');
	P("%c%c%c", 'a', '\t', 'b');
	P(" %c %c ", 'x', 'y');
	P("%c", 0);
	P("a%cb", 0);
	P("%c", 127);
	P("%c", 255);
	P("%c", -1);
	return (1);
}

static int	c_s(int i)
{
	(void)i;
	P("%s", "hola");
	P("%s", "");
	P(" %s ", "");
	P("%s%s%s", "uno", "", "tres");
	P("[%s] [%s]", "a", "bc");
	P("%s", "\xc3\xb1" "and\xc3\xba\t\n");
	P("%s", (char *)NULL);
	P("%s y %s", (char *)NULL, "algo");
	P("%s", g_long);
	return (1);
}

static int	c_p(int i)
{
	int	x;

	(void)i;
	P("%p", (void *)&x);
	P("%p", (void *)g_long);
	P("%p", (void *)NULL);
	P("%p %p", (void *)NULL, (void *)NULL);
	P("%p", (void *)1);
	P("%p", (void *)15);
	P("%p", (void *)16);
	P("%p", (void *)0x7fffffff);
	P("%p", (void *)LONG_MAX);
	P("%p", (void *)LONG_MIN);
	P("%p", (void *)ULONG_MAX);
	return (1);
}

static int	c_d(int i)
{
	(void)i;
	P("%d", 0);
	P("%d", 1);
	P("%d", -1);
	P("%d", 9);
	P("%d", 10);
	P("%d", -10);
	P("%d", 42);
	P("%d", -42);
	P("%d", 100);
	P("%d", 1000000);
	P("%d", INT_MAX);
	P("%d", INT_MIN);
	P("%d", INT_MIN + 1);
	P("[%d][%d][%d]", 1, -2, 3);
	return (1);
}

static int	c_i(int i)
{
	(void)i;
	P("%i", 0);
	P("%i", 7);
	P("%i", -7);
	P("%i", 2147483647);
	P("%i", INT_MIN);
	P("%i %d %i", 1, 2, 3);
	return (1);
}

static int	c_u(int i)
{
	(void)i;
	P("%u", 0u);
	P("%u", 1u);
	P("%u", 42u);
	P("%u", 4294967295u);
	P("%u", (unsigned int)-1);
	P("%u", (unsigned int)-42);
	P("%u", (unsigned int)INT_MIN);
	P("%u", (unsigned int)INT_MAX + 1u);
	return (1);
}

static int	c_x(int i)
{
	(void)i;
	P("%x", 0u);
	P("%x", 1u);
	P("%x", 9u);
	P("%x", 10u);
	P("%x", 15u);
	P("%x", 16u);
	P("%x", 255u);
	P("%x", 0xdeadbeefu);
	P("%x", UINT_MAX);
	P("%x", (unsigned int)-42);
	P("%x", (unsigned int)INT_MIN);
	return (1);
}

static int	c_upx(int i)
{
	(void)i;
	P("%X", 0u);
	P("%X", 10u);
	P("%X", 255u);
	P("%X", 0xdeadbeefu);
	P("%X", UINT_MAX);
	P("%X", (unsigned int)INT_MIN);
	P("%x %X", 0xabcu, 0xabcu);
	return (1);
}

static int	c_pct(int i)
{
	(void)i;
	P("%%");
	P("%%%%");
	P("%%d");
	P("100%%");
	P("%% %c %%", 'a');
	return (1);
}

static int	c_mix(int i)
{
	int	x;

	(void)i;
	P("%c|%s|%p|%d|%i|%u|%x|%X|%%", 'z', "hola", (void *)&x, -42, 42, 42u,
		0xffu, 0xffu);
	P("%s tiene %d anos y %u%% de %c", "Ana", 30, 50u, 'x');
	P("%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d", 1, 2, 3, 4, 5, 6, 7, 8, 9,
		10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20);
	P("%s%s%s%s%s%s%s%s", "a", "b", "c", "d", "e", "f", "g", "h");
	P("%x%c%X%s%u", 3054u, '\n', 3054u, "\n", 0u);
	return (1);
}

/* Varias llamadas seguidas: que no arrastre nada de una a otra (statics). */
static int	c_seq(int i)
{
	(void)i;
	for (int k = 0; k < 50; k++)
	{
		P("%d", k * 37 - 900);
		P("%s", k % 2 ? "par" : "");
		P("%x", (unsigned int)k * 0x1001u);
	}
	return (1);
}

/* ---------------------------- malloc que falla ---------------------------- */

static int	f_fail(void)
{
	int	x;

	cap_begin();
	ft_printf("%s|%d|%x|%p|%u|%c|%X|%%|%i\n", "hola", -42, 255u, (void *)&x,
		42u, 'z', 0xabcu, INT_MIN);
	cap_end();
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

/* Cuantos malloc hace una llamada normal, contados en un hijo.            */
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

/* Hasta 12 puntos de fallo repartidos entre el primer malloc y el ultimo.  */
/* Si ft_printf no usa malloc no sale nada: no hay nada que probar.          */
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
	group_begin("malloc", "un malloc que falla, en cada punto");
	for (long j = 0; j < m; j++)
	{
		k = m == 1 ? 1 : 1 + j * (n - 1) / (m - 1);
		snprintf(nm, sizeof(nm), "con el malloc n\xc2\xba %ld de %ld fallando:"
			" no revienta ni pierde memoria", k, n);
		run(nm, c_fail, (int)k, -1);
	}
	group_end();
}

/* ------------------------------ casos bonus ------------------------------- */

static int	c_minus(int i)
{
	(void)i;
	P("[%-5c]", 'a');
	P("[%-1c]", 'a');
	P("[%-5s]", "ab");
	P("[%-2s]", "abcd");
	P("[%-8s]", (char *)NULL);
	P("[%-6d]", 42);
	P("[%-6d]", -42);
	P("[%-1d]", 12345);
	P("[%-12i]", INT_MIN);
	P("[%-6u]", 42u);
	P("[%-6x]", 255u);
	P("[%-6X]", 255u);
	P("[%-20p]", (void *)0x1234);
	P("[%-8p]", (void *)NULL);
	return (1);
}

static int	c_width(int i)
{
	(void)i;
	P("[%5c]", 'a');
	P("[%1c]", 'a');
	P("[%3c]", 0);
	P("[%10s]", "hola");
	P("[%2s]", "hola");
	P("[%10s]", (char *)NULL);
	P("[%5d]", 42);
	P("[%5d]", -42);
	P("[%2d]", 12345);
	P("[%15d]", INT_MIN);
	P("[%5u]", 7u);
	P("[%5x]", 255u);
	P("[%5X]", 255u);
	P("[%20p]", (void *)0x1234);
	P("[%10p]", (void *)NULL);
	return (1);
}

static int	c_zero(int i)
{
	(void)i;
	P("[%05d]", 42);
	P("[%05d]", -42);
	P("[%01d]", -42);
	P("[%012d]", INT_MIN);
	P("[%05i]", 0);
	P("[%05u]", 42u);
	P("[%08x]", 0xbeefu);
	P("[%08X]", 0xbeefu);
	P("[%03x]", 0u);
	return (1);
}

static int	c_prec(int i)
{
	(void)i;
	P("[%.3d]", 7);
	P("[%.3d]", -7);
	P("[%.5d]", -42);
	P("[%.1d]", 12345);
	P("[%.0d]", 0);
	P("[%.d]", 0);
	P("[%.0d]", 5);
	P("[%.12d]", INT_MIN);
	P("[%.3i]", 1);
	P("[%.4u]", 9u);
	P("[%.0u]", 0u);
	P("[%.6x]", 0xabu);
	P("[%.0x]", 0u);
	P("[%.6X]", 0xabu);
	P("[%.3s]", "hola");
	P("[%.0s]", "hola");
	P("[%.10s]", "hola");
	P("[%.s]", "hola");
	return (1);
}

static int	c_mixwp(int i)
{
	(void)i;
	P("[%8.3d]", 7);
	P("[%8.3d]", -7);
	P("[%-8.3d]", -7);
	P("[%08.3d]", 7);
	P("[%08.3d]", -7);
	P("[%5.0d]", 0);
	P("[%-5.0d]", 0);
	P("[%3.5d]", 42);
	P("[%10.4u]", 42u);
	P("[%010.4u]", 42u);
	P("[%8.4x]", 0xau);
	P("[%-8.4X]", 0xau);
	P("[%6.2s]", "hola");
	P("[%-6.2s]", "hola");
	P("[%-5.0s]", "hola");
	return (1);
}

static int	c_hash(int i)
{
	(void)i;
	P("[%#x]", 0u);
	P("[%#x]", 42u);
	P("[%#X]", 42u);
	P("[%#x]", UINT_MAX);
	P("[%#8x]", 42u);
	P("[%#-8x]", 42u);
	P("[%#08x]", 42u);
	P("[%#08X]", 42u);
	P("[%#.5x]", 42u);
	P("[%#10.5X]", 42u);
	P("[%#.0x]", 0u);
	P("[%#5x]", 0u);
	return (1);
}

static int	c_plus(int i)
{
	(void)i;
	P("[%+d]", 0);
	P("[%+d]", 42);
	P("[%+d]", -42);
	P("[%+i]", INT_MAX);
	P("[%+d]", INT_MIN);
	P("[%+5d]", 42);
	P("[%+-5d]", 42);
	P("[%+05d]", 42);
	P("[%+05d]", -42);
	P("[%+.3d]", 7);
	P("[%+.0d]", 0);
	P("[%+6.3i]", 7);
	return (1);
}

static int	c_space(int i)
{
	(void)i;
	P("[% d]", 0);
	P("[% d]", 42);
	P("[% d]", -42);
	P("[% i]", INT_MAX);
	P("[% d]", INT_MIN);
	P("[% 5d]", 42);
	P("[% -5d]", 42);
	P("[% 05d]", 42);
	P("[% .3d]", 7);
	P("[% .0d]", 0);
	P("[% +d]", 42);
	P("[%+ d]", 42);
	return (1);
}

/* '-' gana a '0' y una precision anula el '0' en los enteros.             */
static int	c_combo(int i)
{
	(void)i;
	P("[%-05d]", 42);
	P("[%0-5d]", -42);
	P("[%-#8x]", 42u);
	P("[%- 6d]", 42);
	P("[%-+6d]", -42);
	P("[%5c|%-5s|%05d|%.3x|%#X|%+d|% i|%-15p]", 'a', "ab", -7, 5u, 255u, 3, 4,
		(void *)0xbeef);
	return (1);
}

/* -------------------------------- suites ---------------------------------- */

static void	suite_mand(void)
{
	each("texto", "sin conversiones", c_text);
	each("%c", "un caracter", c_c);
	each("%s", "una cadena", c_s);
	each("%p", "un puntero void * en hexadecimal", c_p);
	each("%d", "un numero decimal (base 10)", c_d);
	each("%i", "un entero en base 10", c_i);
	each("%u", "un numero decimal sin signo (base 10)", c_u);
	each("%x", "un hexadecimal (base 16) en minusculas", c_x);
	each("%X", "un hexadecimal (base 16) en mayusculas", c_upx);
	each("%%", "el simbolo del porcentaje", c_pct);
	each("mezcla", "todas las conversiones juntas", c_mix);
	group_begin("seguidas", "150 llamadas sin arrastrar nada");
	run("150 llamadas seguidas (que no arrastre nada de una a otra)", c_seq,
		0, -1);
	group_end();
	fail_sweep();
}

static void	suite_bonus(void)
{
	each("-", "alinear a la izquierda", c_minus);
	each("anchura", "anchura minima", c_width);
	each("0", "rellenar con ceros", c_zero);
	each(".", "precision", c_prec);
	each("anch.prec", "anchura y precision juntas", c_mixwp);
	each("#", "0x / 0X delante", c_hash);
	each("+", "signo siempre", c_plus);
	each("' '", "espacio en lugar del +", c_space);
	each("combo", "flags combinados", c_combo);
}

int	main(int argc, char **argv)
{
	char	path[4200];

	init_colors();
	if (argc != 3)
		return (2);
	mkdir(argv[2], 0755);
	snprintf(path, sizeof(path), "%s/cap.XXXXXX", argv[2]);
	g_cap = mkstemp(path);
	g_out = dup(STDOUT_FILENO);
	if (g_cap < 0 || g_out < 0)
		return (perror(path), 1);
	unlink(path);
	g_long = __libc_malloc(20001);
	for (int k = 0; k < 20000; k++)
		g_long[k] = (char)('A' + k % 26);
	g_long[20000] = '\0';
	g_o = stdout;
	g_quiet = !strcmp(argv[1], "bonusmand");
	if (!strcmp(argv[1], "bonus"))
		suite_bonus();
	else
		suite_mand();
	if (g_quiet)
		printf("  %s%d/%d%s casos obligatorios OK con la " "libftprintf.a de"
			" make bonus\n\n", g_nko ? C_KO : C_OK, g_nok, g_nok + g_nko, C_0);
	else
		summary(strcmp(argv[1], "bonus") ? "resumen" : "resumen bonus");
	return (g_nko != 0);
}

#endif
