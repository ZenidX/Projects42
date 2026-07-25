#!/usr/bin/env python3
"""Genera C11 (punteros a funcion), C12 (listas) y C13 (arboles binarios).

Cuerpos conformes a la Norma: header 42, tabs, declaraciones al inicio + linea
en blanco, una por linea, sin asignar en la declaracion, sin for/switch/goto,
<=25 lineas por funcion, <=5 funciones por fichero, include guards en los .h.
Uso: wsl -d Ubuntu -- python3 /mnt/e/WORK/Xavi/Projects42/_build/gen_c11_13.py
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from header42 import header, write_file

ROOT = "/mnt/e/WORK/Xavi/Projects42/c"

# Cabeceras compartidas -------------------------------------------------------
FT_LIST_H = """#ifndef FT_LIST_H
# define FT_LIST_H

typedef struct s_list
{
\tstruct s_list\t*next;
\tvoid\t\t\t*data;
}\tt_list;

t_list\t*ft_create_elem(void *data);

#endif
"""

FT_BTREE_H = """#ifndef FT_BTREE_H
# define FT_BTREE_H

typedef struct s_btree
{
\tstruct s_btree\t*left;
\tstruct s_btree\t*right;
\tvoid\t\t\t*item;
}\tt_btree;

t_btree\t*btree_create_node(void *item);

#endif
"""

F = {}

# ============================== C 11 =========================================
F["c11/ex00/ft_foreach.c"] = """void\tft_foreach(int *tab, int length, void (*f)(int))
{
\tint\ti;

\ti = 0;
\twhile (i < length)
\t{
\t\tf(tab[i]);
\t\ti++;
\t}
}
"""

F["c11/ex01/ft_map.c"] = """#include <stdlib.h>

int\t*ft_map(int *tab, int length, int (*f)(int))
{
\tint\t*res;
\tint\ti;

\tres = malloc(sizeof(int) * length);
\tif (!res)
\t\treturn (NULL);
\ti = 0;
\twhile (i < length)
\t{
\t\tres[i] = f(tab[i]);
\t\ti++;
\t}
\treturn (res);
}
"""

F["c11/ex02/ft_any.c"] = """int\tft_any(char **tab, int (*f)(char *))
{
\tint\ti;

\ti = 0;
\twhile (tab[i])
\t{
\t\tif (f(tab[i]))
\t\t\treturn (1);
\t\ti++;
\t}
\treturn (0);
}
"""

F["c11/ex03/ft_count_if.c"] = """int\tft_count_if(char **tab, int length, int (*f)(char *))
{
\tint\ti;
\tint\tcount;

\ti = 0;
\tcount = 0;
\twhile (i < length)
\t{
\t\tif (f(tab[i]))
\t\t\tcount++;
\t\ti++;
\t}
\treturn (count);
}
"""

F["c11/ex04/ft_is_sort.c"] = """int\tft_is_sort(int *tab, int length, int (*f)(int, int))
{
\tint\ti;
\tint\tup;
\tint\tdown;

\ti = 0;
\tup = 1;
\tdown = 1;
\twhile (i < length - 1)
\t{
\t\tif (f(tab[i], tab[i + 1]) > 0)
\t\t\tup = 0;
\t\tif (f(tab[i], tab[i + 1]) < 0)
\t\t\tdown = 0;
\t\ti++;
\t}
\treturn (up || down);
}
"""

# --- ex05 do-op (programa) ---
F["c11/ex05/do_op.h"] = """#ifndef DO_OP_H
# define DO_OP_H

typedef struct s_op
{
\tchar\t*sym;
\tint\t\t(*f)(int, int);
}\tt_op;

int\t\tft_add(int a, int b);
int\t\tft_sub(int a, int b);
int\t\tft_mul(int a, int b);
int\t\tft_div(int a, int b);
int\t\tft_mod(int a, int b);
int\t\tft_atoi(char *str);
void\tft_putnbr(int nb);

#endif
"""

F["c11/ex05/ops.c"] = """int\tft_add(int a, int b)
{
\treturn (a + b);
}

int\tft_sub(int a, int b)
{
\treturn (a - b);
}

int\tft_mul(int a, int b)
{
\treturn (a * b);
}

int\tft_div(int a, int b)
{
\treturn (a / b);
}

int\tft_mod(int a, int b)
{
\treturn (a % b);
}
"""

F["c11/ex05/ft_atoi.c"] = """int\tft_atoi(char *str)
{
\tint\ti;
\tint\tsign;
\tint\tres;

\ti = 0;
\tsign = 1;
\tres = 0;
\twhile (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
\t\ti++;
\twhile (str[i] == '+' || str[i] == '-')
\t{
\t\tif (str[i] == '-')
\t\t\tsign = -sign;
\t\ti++;
\t}
\twhile (str[i] >= '0' && str[i] <= '9')
\t{
\t\tres = res * 10 + (str[i] - '0');
\t\ti++;
\t}
\treturn (res * sign);
}
"""

F["c11/ex05/ft_putnbr.c"] = """#include <unistd.h>

void\tft_putnbr(int nb)
{
\tchar\tc;
\tlong\tn;

\tn = nb;
\tif (n < 0)
\t{
\t\twrite(1, "-", 1);
\t\tn = -n;
\t}
\tif (n >= 10)
\t\tft_putnbr(n / 10);
\tc = n % 10 + '0';
\twrite(1, &c, 1);
}
"""

F["c11/ex05/do_op.c"] = """#include <unistd.h>
#include "do_op.h"

static int\tft_strcmp(char *a, char *b)
{
\tint\ti;

\ti = 0;
\twhile (a[i] && a[i] == b[i])
\t\ti++;
\treturn (a[i] - b[i]);
}

static int\tis_zero_err(char *op, int b)
{
\tif (b != 0)
\t\treturn (0);
\tif (!ft_strcmp(op, "/"))
\t{
\t\twrite(1, "Stop : division by zero\\n", 24);
\t\treturn (1);
\t}
\tif (!ft_strcmp(op, "%"))
\t{
\t\twrite(1, "Stop : modulo by zero\\n", 22);
\t\treturn (1);
\t}
\treturn (0);
}

static int\trun(char *op, int a, int b)
{
\tt_op\tops[5];
\tint\t\ti;

\tops[0].sym = "+";
\tops[0].f = &ft_add;
\tops[1].sym = "-";
\tops[1].f = &ft_sub;
\tops[2].sym = "*";
\tops[2].f = &ft_mul;
\tops[3].sym = "/";
\tops[3].f = &ft_div;
\tops[4].sym = "%";
\tops[4].f = &ft_mod;
\ti = 0;
\twhile (i < 5)
\t{
\t\tif (!ft_strcmp(ops[i].sym, op))
\t\t\treturn (ops[i].f(a, b));
\t\ti++;
\t}
\treturn (0);
}

int\tmain(int argc, char **argv)
{
\tint\ta;
\tint\tb;

\tif (argc != 4)
\t\treturn (0);
\ta = ft_atoi(argv[1]);
\tb = ft_atoi(argv[3]);
\tif (is_zero_err(argv[2], b))
\t\treturn (0);
\tft_putnbr(run(argv[2], a, b));
\twrite(1, "\\n", 1);
\treturn (0);
}
"""

F["c11/ex06/ft_sort_string_tab.c"] = """static int\tft_strcmp(char *a, char *b)
{
\tint\ti;

\ti = 0;
\twhile (a[i] && a[i] == b[i])
\t\ti++;
\treturn (a[i] - b[i]);
}

void\tft_sort_string_tab(char **tab)
{
\tint\t\ti;
\tchar\t*tmp;

\ti = 0;
\twhile (tab[i] && tab[i + 1])
\t{
\t\tif (ft_strcmp(tab[i], tab[i + 1]) > 0)
\t\t{
\t\t\ttmp = tab[i];
\t\t\ttab[i] = tab[i + 1];
\t\t\ttab[i + 1] = tmp;
\t\t\ti = 0;
\t\t}
\t\telse
\t\t\ti++;
\t}
}
"""

F["c11/ex07/ft_advanced_sort_string_tab.c"] = """void\tft_advanced_sort_string_tab(char **tab, int (*cmp)(char *, char *))
{
\tint\t\ti;
\tchar\t*tmp;

\ti = 0;
\twhile (tab[i] && tab[i + 1])
\t{
\t\tif (cmp(tab[i], tab[i + 1]) > 0)
\t\t{
\t\t\ttmp = tab[i];
\t\t\ttab[i] = tab[i + 1];
\t\t\ttab[i + 1] = tmp;
\t\t\ti = 0;
\t\t}
\t\telse
\t\t\ti++;
\t}
}
"""

# ============================== C 12 =========================================
F["c12/ex00/ft_list.h"] = FT_LIST_H
F["c12/ex00/ft_create_elem.c"] = """#include <stdlib.h>
#include "ft_list.h"

t_list\t*ft_create_elem(void *data)
{
\tt_list\t*elem;

\telem = malloc(sizeof(t_list));
\tif (!elem)
\t\treturn (NULL);
\telem->data = data;
\telem->next = NULL;
\treturn (elem);
}
"""

F["c12/ex01/ft_list.h"] = FT_LIST_H
F["c12/ex01/ft_list_push_front.c"] = """#include "ft_list.h"

void\tft_list_push_front(t_list **begin_list, void *data)
{
\tt_list\t*elem;

\telem = ft_create_elem(data);
\tif (!elem)
\t\treturn ;
\telem->next = *begin_list;
\t*begin_list = elem;
}
"""

F["c12/ex02/ft_list.h"] = FT_LIST_H
F["c12/ex02/ft_list_size.c"] = """#include "ft_list.h"

int\tft_list_size(t_list *begin_list)
{
\tint\tsize;

\tsize = 0;
\twhile (begin_list)
\t{
\t\tsize++;
\t\tbegin_list = begin_list->next;
\t}
\treturn (size);
}
"""

F["c12/ex03/ft_list.h"] = FT_LIST_H
F["c12/ex03/ft_list_last.c"] = """#include <stddef.h>
#include "ft_list.h"

t_list\t*ft_list_last(t_list *begin_list)
{
\tif (!begin_list)
\t\treturn (NULL);
\twhile (begin_list->next)
\t\tbegin_list = begin_list->next;
\treturn (begin_list);
}
"""

F["c12/ex04/ft_list.h"] = FT_LIST_H
F["c12/ex04/ft_list_push_back.c"] = """#include "ft_list.h"

void\tft_list_push_back(t_list **begin_list, void *data)
{
\tt_list\t*elem;
\tt_list\t*last;

\telem = ft_create_elem(data);
\tif (!elem)
\t\treturn ;
\tif (!*begin_list)
\t{
\t\t*begin_list = elem;
\t\treturn ;
\t}
\tlast = *begin_list;
\twhile (last->next)
\t\tlast = last->next;
\tlast->next = elem;
}
"""

F["c12/ex05/ft_list.h"] = FT_LIST_H
F["c12/ex05/ft_list_push_strs.c"] = """#include <stddef.h>
#include "ft_list.h"

t_list\t*ft_list_push_strs(int size, char **strs)
{
\tt_list\t*list;
\tt_list\t*elem;
\tint\t\ti;

\tlist = NULL;
\ti = 0;
\twhile (i < size)
\t{
\t\telem = ft_create_elem(strs[i]);
\t\tif (!elem)
\t\t\treturn (list);
\t\telem->next = list;
\t\tlist = elem;
\t\ti++;
\t}
\treturn (list);
}
"""

F["c12/ex06/ft_list.h"] = FT_LIST_H
F["c12/ex06/ft_list_clear.c"] = """#include <stdlib.h>
#include "ft_list.h"

void\tft_list_clear(t_list *begin_list, void (*free_fct)(void *))
{
\tt_list\t*tmp;

\twhile (begin_list)
\t{
\t\ttmp = begin_list->next;
\t\tfree_fct(begin_list->data);
\t\tfree(begin_list);
\t\tbegin_list = tmp;
\t}
}
"""

F["c12/ex07/ft_list.h"] = FT_LIST_H
F["c12/ex07/ft_list_at.c"] = """#include <stddef.h>
#include "ft_list.h"

t_list\t*ft_list_at(t_list *begin_list, unsigned int nbr)
{
\tunsigned int\ti;

\ti = 0;
\twhile (begin_list)
\t{
\t\tif (i == nbr)
\t\t\treturn (begin_list);
\t\ti++;
\t\tbegin_list = begin_list->next;
\t}
\treturn (NULL);
}
"""

# ex08: solo el .c (usa su propio ft_list.h)
F["c12/ex08/ft_list_reverse.c"] = """#include <stddef.h>
#include "ft_list.h"

void\tft_list_reverse(t_list **begin_list)
{
\tt_list\t*prev;
\tt_list\t*cur;
\tt_list\t*next;

\tprev = NULL;
\tcur = *begin_list;
\twhile (cur)
\t{
\t\tnext = cur->next;
\t\tcur->next = prev;
\t\tprev = cur;
\t\tcur = next;
\t}
\t*begin_list = prev;
}
"""

F["c12/ex09/ft_list.h"] = FT_LIST_H
F["c12/ex09/ft_list_foreach.c"] = """#include "ft_list.h"

void\tft_list_foreach(t_list *begin_list, void (*f)(void *))
{
\twhile (begin_list)
\t{
\t\tf(begin_list->data);
\t\tbegin_list = begin_list->next;
\t}
}
"""

F["c12/ex10/ft_list.h"] = FT_LIST_H
F["c12/ex10/ft_list_foreach_if.c"] = """#include "ft_list.h"

void\tft_list_foreach_if(t_list *begin_list, void (*f)(void *),
\t\tvoid *data_ref, int (*cmp)(void *, void *))
{
\twhile (begin_list)
\t{
\t\tif (cmp(begin_list->data, data_ref) == 0)
\t\t\tf(begin_list->data);
\t\tbegin_list = begin_list->next;
\t}
}
"""

F["c12/ex11/ft_list.h"] = FT_LIST_H
F["c12/ex11/ft_list_find.c"] = """#include <stddef.h>
#include "ft_list.h"

t_list\t*ft_list_find(t_list *begin_list, void *data_ref, int (*cmp)())
{
\twhile (begin_list)
\t{
\t\tif (cmp(begin_list->data, data_ref) == 0)
\t\t\treturn (begin_list);
\t\tbegin_list = begin_list->next;
\t}
\treturn (NULL);
}
"""

F["c12/ex12/ft_list.h"] = FT_LIST_H
F["c12/ex12/ft_list_remove_if.c"] = """#include <stdlib.h>
#include "ft_list.h"

void\tft_list_remove_if(t_list **begin_list, void *data_ref,
\t\tint (*cmp)(), void (*free_fct)(void *))
{
\tt_list\t*cur;
\tt_list\t*tmp;

\twhile (*begin_list && cmp((*begin_list)->data, data_ref) == 0)
\t{
\t\ttmp = (*begin_list)->next;
\t\tfree_fct((*begin_list)->data);
\t\tfree(*begin_list);
\t\t*begin_list = tmp;
\t}
\tcur = *begin_list;
\twhile (cur && cur->next)
\t{
\t\tif (cmp(cur->next->data, data_ref) == 0)
\t\t{
\t\t\ttmp = cur->next;
\t\t\tcur->next = tmp->next;
\t\t\tfree_fct(tmp->data);
\t\t\tfree(tmp);
\t\t}
\t\telse
\t\t\tcur = cur->next;
\t}
}
"""

F["c12/ex13/ft_list.h"] = FT_LIST_H
F["c12/ex13/ft_list_merge.c"] = """#include "ft_list.h"

void\tft_list_merge(t_list **begin_list1, t_list *begin_list2)
{
\tt_list\t*last;

\tif (!*begin_list1)
\t{
\t\t*begin_list1 = begin_list2;
\t\treturn ;
\t}
\tlast = *begin_list1;
\twhile (last->next)
\t\tlast = last->next;
\tlast->next = begin_list2;
}
"""

F["c12/ex14/ft_list.h"] = FT_LIST_H
F["c12/ex14/ft_list_sort.c"] = """#include "ft_list.h"

void\tft_list_sort(t_list **begin_list, int (*cmp)())
{
\tt_list\t*cur;
\tvoid\t*tmp;

\tif (!begin_list || !*begin_list)
\t\treturn ;
\tcur = *begin_list;
\twhile (cur->next)
\t{
\t\tif (cmp(cur->data, cur->next->data) > 0)
\t\t{
\t\t\ttmp = cur->data;
\t\t\tcur->data = cur->next->data;
\t\t\tcur->next->data = tmp;
\t\t\tcur = *begin_list;
\t\t}
\t\telse
\t\t\tcur = cur->next;
\t}
}
"""

F["c12/ex15/ft_list.h"] = FT_LIST_H
F["c12/ex15/ft_list_reverse_fun.c"] = """#include "ft_list.h"

static int\tlist_len(t_list *l)
{
\tint\tn;

\tn = 0;
\twhile (l)
\t{
\t\tn++;
\t\tl = l->next;
\t}
\treturn (n);
}

static t_list\t*node_at(t_list *begin, int pos)
{
\tint\ti;

\ti = 0;
\twhile (i < pos)
\t{
\t\tbegin = begin->next;
\t\ti++;
\t}
\treturn (begin);
}

void\tft_list_reverse_fun(t_list *begin_list)
{
\tt_list\t*left;
\tt_list\t*right;
\tvoid\t*tmp;
\tint\t\ti;
\tint\t\tn;

\tn = list_len(begin_list);
\tleft = begin_list;
\ti = 0;
\twhile (i < n / 2)
\t{
\t\tright = node_at(begin_list, n - 1 - i);
\t\ttmp = left->data;
\t\tleft->data = right->data;
\t\tright->data = tmp;
\t\tleft = left->next;
\t\ti++;
\t}
}
"""

F["c12/ex16/ft_list.h"] = FT_LIST_H
F["c12/ex16/ft_sorted_list_insert.c"] = """#include "ft_list.h"

void\tft_sorted_list_insert(t_list **begin_list, void *data, int (*cmp)())
{
\tt_list\t*elem;
\tt_list\t*cur;

\telem = ft_create_elem(data);
\tif (!elem)
\t\treturn ;
\tif (!*begin_list || cmp((*begin_list)->data, data) > 0)
\t{
\t\telem->next = *begin_list;
\t\t*begin_list = elem;
\t\treturn ;
\t}
\tcur = *begin_list;
\twhile (cur->next && cmp(cur->next->data, data) <= 0)
\t\tcur = cur->next;
\telem->next = cur->next;
\tcur->next = elem;
}
"""

F["c12/ex17/ft_list.h"] = FT_LIST_H
F["c12/ex17/ft_sorted_list_merge.c"] = """#include "ft_list.h"

static void\tins_sorted(t_list **head, t_list *node, int (*cmp)())
{
\tt_list\t*cur;

\tif (!*head || cmp((*head)->data, node->data) > 0)
\t{
\t\tnode->next = *head;
\t\t*head = node;
\t\treturn ;
\t}
\tcur = *head;
\twhile (cur->next && cmp(cur->next->data, node->data) <= 0)
\t\tcur = cur->next;
\tnode->next = cur->next;
\tcur->next = node;
}

void\tft_sorted_list_merge(t_list **begin_list1, t_list *begin_list2,
\t\tint (*cmp)())
{
\tt_list\t*next2;

\twhile (begin_list2)
\t{
\t\tnext2 = begin_list2->next;
\t\tins_sorted(begin_list1, begin_list2, cmp);
\t\tbegin_list2 = next2;
\t}
}
"""

# ============================== C 13 =========================================
F["c13/ex00/ft_btree.h"] = FT_BTREE_H
F["c13/ex00/btree_create_node.c"] = """#include <stdlib.h>
#include "ft_btree.h"

t_btree\t*btree_create_node(void *item)
{
\tt_btree\t*node;

\tnode = malloc(sizeof(t_btree));
\tif (!node)
\t\treturn (NULL);
\tnode->left = NULL;
\tnode->right = NULL;
\tnode->item = item;
\treturn (node);
}
"""

F["c13/ex01/ft_btree.h"] = FT_BTREE_H
F["c13/ex01/btree_apply_prefix.c"] = """#include "ft_btree.h"

void\tbtree_apply_prefix(t_btree *root, void (*applyf)(void *))
{
\tif (!root)
\t\treturn ;
\tapplyf(root->item);
\tbtree_apply_prefix(root->left, applyf);
\tbtree_apply_prefix(root->right, applyf);
}
"""

F["c13/ex02/ft_btree.h"] = FT_BTREE_H
F["c13/ex02/btree_apply_infix.c"] = """#include "ft_btree.h"

void\tbtree_apply_infix(t_btree *root, void (*applyf)(void *))
{
\tif (!root)
\t\treturn ;
\tbtree_apply_infix(root->left, applyf);
\tapplyf(root->item);
\tbtree_apply_infix(root->right, applyf);
}
"""

F["c13/ex03/ft_btree.h"] = FT_BTREE_H
F["c13/ex03/btree_apply_suffix.c"] = """#include "ft_btree.h"

void\tbtree_apply_suffix(t_btree *root, void (*applyf)(void *))
{
\tif (!root)
\t\treturn ;
\tbtree_apply_suffix(root->left, applyf);
\tbtree_apply_suffix(root->right, applyf);
\tapplyf(root->item);
}
"""

F["c13/ex04/ft_btree.h"] = FT_BTREE_H
F["c13/ex04/btree_insert_data.c"] = """#include "ft_btree.h"

void\tbtree_insert_data(t_btree **root, void *item,
\t\tint (*cmpf)(void *, void *))
{
\tif (!*root)
\t{
\t\t*root = btree_create_node(item);
\t\treturn ;
\t}
\tif (cmpf(item, (*root)->item) < 0)
\t\tbtree_insert_data(&(*root)->left, item, cmpf);
\telse
\t\tbtree_insert_data(&(*root)->right, item, cmpf);
}
"""

F["c13/ex05/ft_btree.h"] = FT_BTREE_H
F["c13/ex05/btree_search_item.c"] = """#include <stddef.h>
#include "ft_btree.h"

void\t*btree_search_item(t_btree *root, void *data_ref,
\t\tint (*cmpf)(void *, void *))
{
\tvoid\t*found;

\tif (!root)
\t\treturn (NULL);
\tfound = btree_search_item(root->left, data_ref, cmpf);
\tif (found)
\t\treturn (found);
\tif (cmpf(root->item, data_ref) == 0)
\t\treturn (root->item);
\treturn (btree_search_item(root->right, data_ref, cmpf));
}
"""

F["c13/ex06/ft_btree.h"] = FT_BTREE_H
F["c13/ex06/btree_level_count.c"] = """#include "ft_btree.h"

int\tbtree_level_count(t_btree *root)
{
\tint\tleft;
\tint\tright;

\tif (!root)
\t\treturn (0);
\tleft = btree_level_count(root->left);
\tright = btree_level_count(root->right);
\tif (left > right)
\t\treturn (left + 1);
\treturn (right + 1);
}
"""

F["c13/ex07/ft_btree.h"] = FT_BTREE_H.replace("""t_btree\t*btree_create_node(void *item);""", """typedef struct s_queue
{
\tt_btree\t\t\t*node;
\tint\t\t\t\tlevel;
\tstruct s_queue\t*next;
}\tt_queue;

t_btree\t*btree_create_node(void *item);""")
F["c13/ex07/btree_apply_by_level.c"] = """#include <stdlib.h>
#include "ft_btree.h"

static t_queue\t*new_qnode(t_btree *node, int level)
{
\tt_queue\t*q;

\tq = malloc(sizeof(t_queue));
\tif (!q)
\t\treturn (NULL);
\tq->node = node;
\tq->level = level;
\tq->next = NULL;
\treturn (q);
}

static t_queue\t*enqueue_kids(t_queue *tail, t_btree *node, int level)
{
\tt_queue\t*q;

\tif (node->left)
\t{
\t\tq = new_qnode(node->left, level + 1);
\t\tif (!q)
\t\t\treturn (tail);
\t\ttail->next = q;
\t\ttail = q;
\t}
\tif (node->right)
\t{
\t\tq = new_qnode(node->right, level + 1);
\t\tif (!q)
\t\t\treturn (tail);
\t\ttail->next = q;
\t\ttail = q;
\t}
\treturn (tail);
}

void\tbtree_apply_by_level(t_btree *root, void (*applyf)(void *, int, int))
{
\tt_queue\t*head;
\tt_queue\t*tail;
\tt_queue\t*tmp;
\tint\t\tprev;

\tif (!root)
\t\treturn ;
\thead = new_qnode(root, 0);
\ttail = head;
\tprev = -1;
\twhile (head)
\t{
\t\tapplyf(head->node->item, head->level, head->level != prev);
\t\tprev = head->level;
\t\ttail = enqueue_kids(tail, head->node, head->level);
\t\ttmp = head;
\t\thead = head->next;
\t\tfree(tmp);
\t}
}
"""

# Makefile del do-op (sin header C: rompe make; norminette no lo evalua) -------
MAKEFILE = """# do-op - C Piscine C11 ex05
NAME = do-op
SRCS = do_op.c ops.c ft_atoi.c ft_putnbr.c
OBJS = $(SRCS:.c=.o)
CC = cc
CFLAGS = -Wall -Wextra -Werror

all: $(NAME)

$(NAME): $(OBJS)
\t$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

%.o: %.c do_op.h
\t$(CC) $(CFLAGS) -c $< -o $@

clean:
\trm -f $(OBJS)

fclean: clean
\trm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
"""


def main():
    for rel, body in F.items():
        write_file(ROOT, rel, body)
    mk = os.path.join(ROOT, "c11/ex05/Makefile")
    os.makedirs(os.path.dirname(mk), exist_ok=True)
    with open(mk, "w", newline="\n") as f:
        f.write(MAKEFILE)
    print("OK c11/ex05/Makefile")
    print("\n%d archivos generados (+ Makefile)" % len(F))


if __name__ == "__main__":
    main()
