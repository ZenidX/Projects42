/* ************************************************************************** */
/*                                                                            */
/*   test.c — runner de tests para los ejercicios de C12 (listas enlazadas)   */
/*                                                                            */
/*   Compilar:  cc -Wall -Wextra -Werror -o test/runner test/test.c           */
/*   Ejecutar:  ./test/runner [ruta_repo]                                     */
/*                                                                            */
/*   Los ejercicios de C12 son FUNCIONES, no programas: por cada caso se      */
/*   escribe un main de test en un directorio temporal, se compila junto a    */
/*   las fuentes del repo con -Wall -Wextra -Werror (y -I ex00 para          */
/*   ft_list.h), se ejecuta con fork+execv y se compara el stdout capturado   */
/*   byte a byte con lo esperado.                                             */
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

/* Los mains reproducen los del script de referencia (_build/test_c11_13.sh),
   incluidos los helpers mk() (lista sobre array estatico) y mkm() (lista con
   datos malloc'eados para los tests que liberan memoria).                    */

static t_case	g_cases[] = {
	{"ex00", "ft_create_elem", "ex00/ft_create_elem.c", "ex00",
		"#include <stdio.h>\n"
		"#include \"ft_list.h\"\n"
		"int main(void){ int x=42; t_list *e=ft_create_elem(&x);\n"
		"printf(\"%d %d\", *(int*)e->data, e->next==0); return 0; }\n",
		"42 1", NULL},

	{"ex01", "push_front", "ex01/ft_list_push_front.c ex00/ft_create_elem.c",
		"ex00",
		"#include <stdio.h>\n"
		"#include \"ft_list.h\"\n"
		"void ft_list_push_front(t_list**,void*);\n"
		"int main(void){ t_list *l=0; static int v[3]={1,2,3}; int i=0;\n"
		"while(i<3){ft_list_push_front(&l,&v[i]);i++;}\n"
		"while(l){printf(\"%d,\",*(int*)l->data);l=l->next;} return 0; }\n",
		"3,2,1,", NULL},

	{"ex02", "size",
		"ex02/ft_list_size.c ex01/ft_list_push_front.c ex00/ft_create_elem.c",
		"ex00",
		"#include <stdio.h>\n"
		"#include \"ft_list.h\"\n"
		"int ft_list_size(t_list*);\n"
		"void ft_list_push_front(t_list**,void*);\n"
		"int main(void){ t_list *l=0; static int v[3]={1,2,3}; int i=0;\n"
		"while(i<3){ft_list_push_front(&l,&v[i]);i++;}\n"
		"printf(\"%d%d\", ft_list_size(l), ft_list_size(0)); return 0; }\n",
		"30", NULL},

	{"ex03", "last", "ex03/ft_list_last.c ex00/ft_create_elem.c", "ex00",
		"#include <stdio.h>\n"
		"#include \"ft_list.h\"\n"
		"t_list *ft_list_last(t_list*);\n"
		"static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;\n"
		"while(i<n){e=ft_create_elem(&a[i]);"
		"if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}\n"
		"int main(void){ int a[3]={1,2,3}; t_list*l=mk(a,3); "
		"t_list*last=ft_list_last(l);\n"
		"printf(\"%d %d\", *(int*)last->data, last->next==0); return 0; }\n",
		"3 1", NULL},

	{"ex04", "push_back", "ex04/ft_list_push_back.c ex00/ft_create_elem.c",
		"ex00",
		"#include <stdio.h>\n"
		"#include \"ft_list.h\"\n"
		"void ft_list_push_back(t_list**,void*);\n"
		"int main(void){ t_list *l=0; static int v[3]={1,2,3}; int i=0;\n"
		"while(i<3){ft_list_push_back(&l,&v[i]);i++;}\n"
		"while(l){printf(\"%d,\",*(int*)l->data);l=l->next;} return 0; }\n",
		"1,2,3,", NULL},

	{"ex05", "push_strs", "ex05/ft_list_push_strs.c ex00/ft_create_elem.c",
		"ex00",
		"#include <stdio.h>\n"
		"#include \"ft_list.h\"\n"
		"t_list *ft_list_push_strs(int,char**);\n"
		"int main(void){ char *s[]={\"a\",\"b\",\"c\"}; "
		"t_list*l=ft_list_push_strs(3,s);\n"
		"while(l){printf(\"%s,\",(char*)l->data);l=l->next;} return 0; }\n",
		"c,b,a,", NULL},

	{"ex06", "clear", "ex06/ft_list_clear.c ex00/ft_create_elem.c", "ex00",
		"#include <stdio.h>\n"
		"#include <stdlib.h>\n"
		"#include \"ft_list.h\"\n"
		"void ft_list_clear(t_list*,void(*)(void*));\n"
		"int g=0;\n"
		"void ff(void*p){ g++; free(p); }\n"
		"static t_list *mkm(int*a,int n){t_list*h=0,*t=0,*e;int i=0;\n"
		"while(i<n){int*d=malloc(sizeof(int));*d=a[i];e=ft_create_elem(d);\n"
		"if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}\n"
		"int main(void){ int a[3]={1,2,3}; t_list*l=mkm(a,3);\n"
		"ft_list_clear(l,&ff); printf(\"%d\",g); return 0; }\n",
		"3", NULL},

	{"ex07", "at", "ex07/ft_list_at.c ex00/ft_create_elem.c", "ex00",
		"#include <stdio.h>\n"
		"#include \"ft_list.h\"\n"
		"t_list *ft_list_at(t_list*,unsigned int);\n"
		"static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;\n"
		"while(i<n){e=ft_create_elem(&a[i]);"
		"if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}\n"
		"int main(void){ int a[3]={10,20,30}; t_list*l=mk(a,3);\n"
		"printf(\"%d %d %d\", *(int*)ft_list_at(l,0)->data, "
		"*(int*)ft_list_at(l,2)->data,\n"
		"ft_list_at(l,5)==0); return 0; }\n",
		"10 30 1", NULL},

	{"ex08", "reverse", "ex08/ft_list_reverse.c ex00/ft_create_elem.c",
		"ex00",
		"#include <stdio.h>\n"
		"#include \"ft_list.h\"\n"
		"void ft_list_reverse(t_list**);\n"
		"static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;\n"
		"while(i<n){e=ft_create_elem(&a[i]);"
		"if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}\n"
		"int main(void){ int a[3]={1,2,3}; t_list*l=mk(a,3); "
		"ft_list_reverse(&l);\n"
		"while(l){printf(\"%d,\",*(int*)l->data);l=l->next;} return 0; }\n",
		"3,2,1,", NULL},

	{"ex09", "foreach", "ex09/ft_list_foreach.c ex00/ft_create_elem.c",
		"ex00",
		"#include <stdio.h>\n"
		"#include \"ft_list.h\"\n"
		"void ft_list_foreach(t_list*,void(*)(void*));\n"
		"void pr(void*p){ printf(\"%d,\",*(int*)p); }\n"
		"static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;\n"
		"while(i<n){e=ft_create_elem(&a[i]);"
		"if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}\n"
		"int main(void){ int a[3]={1,2,3}; t_list*l=mk(a,3); "
		"ft_list_foreach(l,&pr);\n"
		"return 0; }\n",
		"1,2,3,", NULL},

	{"ex10", "foreach_if", "ex10/ft_list_foreach_if.c ex00/ft_create_elem.c",
		"ex00",
		"#include <stdio.h>\n"
		"#include \"ft_list.h\"\n"
		"void ft_list_foreach_if(t_list*,void(*)(void*),void*,"
		"int(*)(void*,void*));\n"
		"void pr(void*p){ printf(\"%d,\",*(int*)p); }\n"
		"int icmp(void*a,void*b){ return *(int*)a-*(int*)b; }\n"
		"static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;\n"
		"while(i<n){e=ft_create_elem(&a[i]);"
		"if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}\n"
		"int main(void){ int a[4]={1,2,2,3}; int r=2; t_list*l=mk(a,4);\n"
		"ft_list_foreach_if(l,&pr,&r,&icmp); return 0; }\n",
		"2,2,", NULL},

	{"ex11", "find", "ex11/ft_list_find.c ex00/ft_create_elem.c", "ex00",
		"#include <stdio.h>\n"
		"#include \"ft_list.h\"\n"
		"t_list *ft_list_find(t_list*,void*,int(*)());\n"
		"int icmp(void*a,void*b){ return *(int*)a-*(int*)b; }\n"
		"static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;\n"
		"while(i<n){e=ft_create_elem(&a[i]);"
		"if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}\n"
		"int main(void){ int a[4]={1,2,3,2}; int r=2; int r9=9; "
		"t_list*l=mk(a,4);\n"
		"printf(\"%d\", *(int*)ft_list_find(l,&r,&icmp)->data);\n"
		"printf(\"%d\", ft_list_find(l,&r9,&icmp)==0); return 0; }\n",
		"21", NULL},

	{"ex12", "remove_if", "ex12/ft_list_remove_if.c ex00/ft_create_elem.c",
		"ex00",
		"#include <stdio.h>\n"
		"#include <stdlib.h>\n"
		"#include \"ft_list.h\"\n"
		"void ft_list_remove_if(t_list**,void*,int(*)(),void(*)(void*));\n"
		"int icmp(void*a,void*b){ return *(int*)a-*(int*)b; }\n"
		"static t_list *mkm(int*a,int n){t_list*h=0,*t=0,*e;int i=0;\n"
		"while(i<n){int*d=malloc(sizeof(int));*d=a[i];e=ft_create_elem(d);\n"
		"if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}\n"
		"int main(void){ int a[5]={1,2,3,2,2}; int r=2; t_list*l=mkm(a,5);\n"
		"ft_list_remove_if(&l,&r,&icmp,&free);\n"
		"while(l){printf(\"%d,\",*(int*)l->data);l=l->next;} return 0; }\n",
		"1,3,", NULL},

	{"ex13", "merge", "ex13/ft_list_merge.c ex00/ft_create_elem.c", "ex00",
		"#include <stdio.h>\n"
		"#include \"ft_list.h\"\n"
		"void ft_list_merge(t_list**,t_list*);\n"
		"static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;\n"
		"while(i<n){e=ft_create_elem(&a[i]);"
		"if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}\n"
		"int main(void){ int x[3]={1,2,3}; int y[3]={4,5,6}; "
		"t_list*a=mk(x,3);\n"
		"t_list*b=mk(y,3); ft_list_merge(&a,b);\n"
		"while(a){printf(\"%d,\",*(int*)a->data);a=a->next;} return 0; }\n",
		"1,2,3,4,5,6,", NULL},

	{"ex14", "sort", "ex14/ft_list_sort.c ex00/ft_create_elem.c", "ex00",
		"#include <stdio.h>\n"
		"#include \"ft_list.h\"\n"
		"void ft_list_sort(t_list**,int(*)());\n"
		"int icmp(void*a,void*b){ return *(int*)a-*(int*)b; }\n"
		"static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;\n"
		"while(i<n){e=ft_create_elem(&a[i]);"
		"if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}\n"
		"int main(void){ int a[3]={3,1,2}; t_list*l=mk(a,3); "
		"ft_list_sort(&l,&icmp);\n"
		"while(l){printf(\"%d,\",*(int*)l->data);l=l->next;} return 0; }\n",
		"1,2,3,", NULL},

	{"ex15", "reverse_fun", "ex15/ft_list_reverse_fun.c ex00/ft_create_elem.c",
		"ex00",
		"#include <stdio.h>\n"
		"#include \"ft_list.h\"\n"
		"void ft_list_reverse_fun(t_list*);\n"
		"static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;\n"
		"while(i<n){e=ft_create_elem(&a[i]);"
		"if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}\n"
		"int main(void){ int a[4]={1,2,3,4}; t_list*l=mk(a,4); "
		"ft_list_reverse_fun(l);\n"
		"while(l){printf(\"%d,\",*(int*)l->data);l=l->next;} return 0; }\n",
		"4,3,2,1,", NULL},

	{"ex16", "sorted_insert",
		"ex16/ft_sorted_list_insert.c ex00/ft_create_elem.c", "ex00",
		"#include <stdio.h>\n"
		"#include \"ft_list.h\"\n"
		"void ft_sorted_list_insert(t_list**,void*,int(*)());\n"
		"int icmp(void*a,void*b){ return *(int*)a-*(int*)b; }\n"
		"int main(void){ t_list*l=0; static int v[3]={3,1,2}; int i=0;\n"
		"while(i<3){ft_sorted_list_insert(&l,&v[i],&icmp);i++;}\n"
		"while(l){printf(\"%d,\",*(int*)l->data);l=l->next;} return 0; }\n",
		"1,2,3,", NULL},

	{"ex17", "sorted_merge",
		"ex17/ft_sorted_list_merge.c ex00/ft_create_elem.c", "ex00",
		"#include <stdio.h>\n"
		"#include \"ft_list.h\"\n"
		"void ft_sorted_list_merge(t_list**,t_list*,int(*)());\n"
		"int icmp(void*a,void*b){ return *(int*)a-*(int*)b; }\n"
		"static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;\n"
		"while(i<n){e=ft_create_elem(&a[i]);"
		"if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}\n"
		"int main(void){ int x[3]={1,3,5}; int y[3]={2,4,6}; "
		"t_list*a=mk(x,3);\n"
		"t_list*b=mk(y,3); ft_sorted_list_merge(&a,b,&icmp);\n"
		"while(a){printf(\"%d,\",*(int*)a->data);a=a->next;} return 0; }\n",
		"1,2,3,4,5,6,", NULL},
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
	strcpy(g_tmp, "/tmp/c12test.XXXXXX");
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
