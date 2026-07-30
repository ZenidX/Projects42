/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para los ejercicios de C13 (árboles binarios)   */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*                                                                            */
/*   Los ejercicios de C13 son FUNCIONES, no programas: por cada caso se      */
/*   escribe un main de test en un directorio temporal, se compila junto a    */
/*   las fuentes del repo con -Wall -Wextra -Werror (incluyendo siempre       */
/*   ex00/btree_create_node.c y -I ex00 para ft_btree.h), se ejecuta con      */
/*   fork+execv y se compara el stdout capturado byte a byte con lo esperado. */
/*   Los ejercicios cuyo .c no exista se marcan SKIP, no fallan.              */
/*                                                                            */
/* ************************************************************************** */

#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define PATHSZ 4096
#define TIMEOUT 5

/* ------------------------------- tabla de casos -------------------------- */

typedef struct s_case
{
	const char	*ex;        /* directorio del ejercicio (para mensajes)     */
	const char	*desc;
	const char	*srcs;      /* fuentes relativas al repo, sep. por espacio  */
	const char	*inc;       /* dir para -I relativo al repo, o NULL         */
	const char	*main_src;  /* contenido del main.c de test                 */
	const char	*expected;  /* stdout esperado; NULL = usar gen()           */
	char		*(*gen)(void);
}	t_case;

/* Árbol de prueba (balanceado, valores 1..7), igual que $BUILD del shell:    */
/*        4                                                                   */
/*      /   \                                                                 */
/*     2     6                                                                */
/*    / \   / \                                                               */
/*   1   3 5   7                                                              */
#define TREE \
	"static int V[8]={0,1,2,3,4,5,6,7};\n" \
	"t_btree*n1=btree_create_node(&V[1]);\n" \
	"t_btree*n3=btree_create_node(&V[3]);\n" \
	"t_btree*n2=btree_create_node(&V[2]);n2->left=n1;n2->right=n3;\n" \
	"t_btree*n5=btree_create_node(&V[5]);\n" \
	"t_btree*n7=btree_create_node(&V[7]);\n" \
	"t_btree*n6=btree_create_node(&V[6]);n6->left=n5;n6->right=n7;\n" \
	"t_btree*root=btree_create_node(&V[4]);root->left=n2;root->right=n6;"

static t_case	g_cases[] = {
	{"ex00", "create_node", "ex00/btree_create_node.c", "ex00",
		"#include <stdio.h>\n"
		"#include \"ft_btree.h\"\n"
		"int main(void){ int x=7; t_btree*n=btree_create_node(&x);\n"
		"printf(\"%d %d %d\", *(int*)n->item, n->left==0, n->right==0);"
		" return 0; }\n",
		"7 1 1", NULL},

	{"ex01", "apply_prefix",
		"ex01/btree_apply_prefix.c ex00/btree_create_node.c", "ex00",
		"#include <stdio.h>\n"
		"#include \"ft_btree.h\"\n"
		"void btree_apply_prefix(t_btree*,void(*)(void*));\n"
		"void pr(void*p){ printf(\"%d,\",*(int*)p); }\n"
		"int main(void){ " TREE " btree_apply_prefix(root,&pr);"
		" return 0; }\n",
		"4,2,1,3,6,5,7,", NULL},

	{"ex02", "apply_infix",
		"ex02/btree_apply_infix.c ex00/btree_create_node.c", "ex00",
		"#include <stdio.h>\n"
		"#include \"ft_btree.h\"\n"
		"void btree_apply_infix(t_btree*,void(*)(void*));\n"
		"void pr(void*p){ printf(\"%d,\",*(int*)p); }\n"
		"int main(void){ " TREE " btree_apply_infix(root,&pr);"
		" return 0; }\n",
		"1,2,3,4,5,6,7,", NULL},

	{"ex03", "apply_suffix",
		"ex03/btree_apply_suffix.c ex00/btree_create_node.c", "ex00",
		"#include <stdio.h>\n"
		"#include \"ft_btree.h\"\n"
		"void btree_apply_suffix(t_btree*,void(*)(void*));\n"
		"void pr(void*p){ printf(\"%d,\",*(int*)p); }\n"
		"int main(void){ " TREE " btree_apply_suffix(root,&pr);"
		" return 0; }\n",
		"1,3,2,5,7,6,4,", NULL},

	{"ex04", "insert_data",
		"ex04/btree_insert_data.c ex00/btree_create_node.c", "ex00",
		"#include <stdio.h>\n"
		"#include \"ft_btree.h\"\n"
		"void btree_insert_data(t_btree**,void*,int(*)(void*,void*));\n"
		"int icmp(void*a,void*b){ return *(int*)a-*(int*)b; }\n"
		"void ino(t_btree*r){ if(!r)return; ino(r->left);\n"
		"printf(\"%d,\",*(int*)r->item); ino(r->right); }\n"
		"int main(void){ static int v[8]={5,3,8,1,4,7,9,2}; t_btree*root=0;"
		" int i=0;\n"
		"while(i<8){btree_insert_data(&root,&v[i],&icmp);i++;} ino(root);"
		" return 0; }\n",
		"1,2,3,4,5,7,8,9,", NULL},

	{"ex05", "search_item",
		"ex05/btree_search_item.c ex00/btree_create_node.c", "ex00",
		"#include <stdio.h>\n"
		"#include \"ft_btree.h\"\n"
		"void *btree_search_item(t_btree*,void*,int(*)(void*,void*));\n"
		"int icmp(void*a,void*b){ return *(int*)a-*(int*)b; }\n"
		"int main(void){ " TREE " int k=5; int m=99;\n"
		"printf(\"%d %d\", *(int*)btree_search_item(root,&k,&icmp),\n"
		"btree_search_item(root,&m,&icmp)==0); return 0; }\n",
		"5 1", NULL},

	{"ex06", "level_count",
		"ex06/btree_level_count.c ex00/btree_create_node.c", "ex00",
		"#include <stdio.h>\n"
		"#include \"ft_btree.h\"\n"
		"int btree_level_count(t_btree*);\n"
		"int main(void){ " TREE " int s=1;"
		" t_btree*one=btree_create_node(&s);\n"
		"printf(\"%d%d%d\", btree_level_count(root), btree_level_count(one),\n"
		"btree_level_count(0)); return 0; }\n",
		"310", NULL},

	{"ex07", "apply_by_level",
		"ex07/btree_apply_by_level.c ex00/btree_create_node.c", "ex00",
		"#include <stdio.h>\n"
		"#include \"ft_btree.h\"\n"
		"void btree_apply_by_level(t_btree*,void(*)(void*,int,int));\n"
		"void ap(void*it,int lvl,int f){ printf(\"%d:%d:%d \","
		" *(int*)it, lvl, f); }\n"
		"int main(void){ " TREE " btree_apply_by_level(root,&ap);"
		" return 0; }\n",
		"4:0:1 2:1:1 6:1:0 1:2:1 3:2:0 5:2:0 7:2:0 ", NULL},
};
#define N_CASES (sizeof(g_cases) / sizeof(g_cases[0]))

/* --------------------------------- utilidades ---------------------------- */

static const char	*C_OK = "";
static const char	*C_KO = "";
static const char	*C_SK = "";
static const char	*C_D  = "";
static const char	*C_0  = "";

static void	init_colors(void)
{
	if (!isatty(STDOUT_FILENO))
		return ;
	C_OK = "\033[32m";
	C_KO = "\033[31m";
	C_SK = "\033[33m";
	C_D  = "\033[90m";
	C_0  = "\033[0m";
}

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

static void	print_escaped(const char *s)
{
	if (!s)
	{
		printf("(null)");
		return ;
	}
	while (*s)
	{
		if (*s == '\n')
			printf("\\n");
		else if (*s == '\t')
			printf("\\t");
		else if ((unsigned char)*s < 32 || (unsigned char)*s == 127)
			printf("\\x%02x", (unsigned char)*s);
		else
			putchar(*s);
		s++;
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

/* -------------------------------- compilación ---------------------------- */

static char	g_tmp[64];

/* Copia en dst las fuentes del caso prefijadas con el repo y entre
   comillas: "'repo/a.c' 'repo/b.c'". */
static void	build_srcs(char *dst, size_t n, const char *repo, const char *srcs)
{
	size_t	l;
	size_t	tl;
	const char	*p;

	l = 0;
	p = srcs;
	dst[0] = '\0';
	while (*p)
	{
		tl = strcspn(p, " ");
		if (l + tl + strlen(repo) + 8 >= n)
		{
			fprintf(stderr, "error: lista de fuentes demasiado larga\n");
			exit(1);
		}
		l += (size_t)sprintf(dst + l, "%s'%s/%.*s'", l ? " " : "",
				repo, (int)tl, p);
		p += tl;
		while (*p == ' ')
			p++;
	}
}

/* Primera fuente del caso (para access()). */
static void	first_src(char *dst, size_t n, const char *repo, const char *srcs)
{
	size_t	tl;

	tl = strcspn(srcs, " ");
	xsnprintf(dst, n, "%s/%.*s", repo, (int)tl, srcs);
}

/* Compila fuentes + main del caso. 0 ok, 1 error de compilación. */
static int	compile_case(const char *repo, const t_case *c, const char *bin)
{
	char	srcs[PATHSZ * 2];
	char	inc[PATHSZ];
	char	cmd[PATHSZ * 4];
	char	mainp[PATHSZ];
	FILE	*f;

	xsnprintf(mainp, sizeof(mainp), "%s/main.c", g_tmp);
	f = fopen(mainp, "w");
	if (!f)
		return (perror("fopen"), 1);
	fputs(c->main_src, f);
	fclose(f);
	build_srcs(srcs, sizeof(srcs), repo, c->srcs);
	inc[0] = '\0';
	if (c->inc)
		xsnprintf(inc, sizeof(inc), "-I '%s/%s'", repo, c->inc);
	xsnprintf(cmd, sizeof(cmd),
		"cc -Wall -Wextra -Werror %s %s '%s' -o '%s' > '%s/cc.log' 2>&1",
		inc, srcs, mainp, bin, g_tmp);
	return (system(cmd) != 0);
}

static void	dump_log(void)
{
	char	path[PATHSZ];
	char	*log;
	int		fd;

	xsnprintf(path, sizeof(path), "%s/cc.log", g_tmp);
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return ;
	log = read_all(fd);
	close(fd);
	if (log && *log)
		printf("%s%s%s", C_D, log, C_0);
	free(log);
}

/* --------------------------------- ejecución ----------------------------- */

static int	run_bin(const char *bin, char **argv, char **out, int *sig)
{
	int		fds[2];
	int		status;
	pid_t	pid;

	*out = NULL;
	*sig = 0;
	if (pipe(fds) == -1)
		return (-1);
	pid = fork();
	if (pid == -1)
		return (close(fds[0]), close(fds[1]), -1);
	if (pid == 0)
	{
		close(fds[0]);
		dup2(fds[1], STDOUT_FILENO);
		close(fds[1]);
		alarm(TIMEOUT);
		execv(bin, argv);
		_exit(127);
	}
	close(fds[1]);
	*out = read_all(fds[0]);
	close(fds[0]);
	if (waitpid(pid, &status, 0) == -1)
		return (-1);
	if (WIFSIGNALED(status))
		*sig = WTERMSIG(status);
	return (WIFEXITED(status) ? WEXITSTATUS(status) : -1);
}

/* ----------------------------------- main -------------------------------- */

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

static const char	*detect_repo(void)
{
	if (access("repo/ex00", F_OK) == 0)
		return ("repo");
	if (access("ex00", F_OK) == 0)
		return (".");
	if (access("../repo/ex00", F_OK) == 0)
		return ("../repo");
	return ("..");
}

int	main(int argc, char **argv)
{
	const char	*repo;
	int			ok;
	int			ko;
	int			skipped;
	size_t		i;

	repo = (argc > 1) ? argv[1] : detect_repo();
	init_colors();
	strcpy(g_tmp, "/tmp/c13test.XXXXXX");
	if (!mkdtemp(g_tmp))
		return (perror("mkdtemp"), 1);
	printf("repo: %s\n\n", repo);
	ok = 0;
	ko = 0;
	skipped = 0;
	i = 0;
	while (i < N_CASES)
	{
		const t_case	*c = &g_cases[i++];
		char			src0[PATHSZ];
		char			bin[PATHSZ];
		char			*exp;
		char			*out;
		char			*pargv[2];
		int				sig;
		int				code;

		first_src(src0, sizeof(src0), repo, c->srcs);
		if (access(src0, R_OK) != 0)
		{
			skipped++;
			printf("%s[SKIP]%s %s  %s%s%s\n", C_SK, C_0, c->ex, C_D,
				"fuente no encontrado", C_0);
			continue ;
		}
		xsnprintf(bin, sizeof(bin), "%s/prog", g_tmp);
		if (compile_case(repo, c, bin))
		{
			ko++;
			printf("%s[KO]%s   %s  %s %s(no compila)%s\n", C_KO, C_0,
				c->ex, c->desc, C_KO, C_0);
			dump_log();
			continue ;
		}
		pargv[0] = bin;
		pargv[1] = NULL;
		code = run_bin(bin, pargv, &out, &sig);
		exp = c->gen ? c->gen() : NULL;
		if (!sig && out && strcmp(out, c->gen ? exp : c->expected) == 0)
		{
			ok++;
			printf("%s[OK]%s   %s  %s\n", C_OK, C_0, c->ex, c->desc);
		}
		else
		{
			ko++;
			printf("%s[KO]%s   %s  %s\n", C_KO, C_0, c->ex, c->desc);
			printf("       esperado: \"");
			print_escaped(c->gen ? exp : c->expected);
			printf("\"\n       obtenido: \"");
			print_escaped(out);
			printf("\"\n");
			if (sig)
				printf("       %smuere con señal %d (%s)%s\n", C_KO, sig,
					strsignal(sig), C_0);
			else if (code != 0)
				printf("       %ssalida con código %d%s\n", C_D, code, C_0);
		}
		free(exp);
		free(out);
	}
	printf("\n%s%d OK%s  %s%d KO%s  %s%d SKIP%s\n",
		C_OK, ok, C_0, C_KO, ko, C_0, C_SK, skipped, C_0);
	cleanup();
	return (ko != 0);
}
