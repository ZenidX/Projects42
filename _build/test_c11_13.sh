#!/bin/bash
# Compila cada ejercicio de C11/C12/C13 con cc -Wall -Wextra -Werror + un main
# de test, ejecuta y compara la salida exacta. Pensado para el contenedor
# python:3.12 (trae gcc; se enlaza como cc). Raiz de fuentes: C_ROOT o /w/c.
set -u
C=${C_ROOT:-/w/c}
TMP=$(mktemp -d)
PASS=0
FAIL=0
ok()  { echo "PASS: $1"; PASS=$((PASS+1)); }
bad() { echo "FAIL: $1"; FAIL=$((FAIL+1)); }

# t <nombre> <esperado> <main.c> <fuente...>
t() {
  local name=$1 exp=$2 main=$3
  shift 3
  printf '%s\n' "$main" > "$TMP/main.c"
  if ! cc -Wall -Wextra -Werror -I "$C/c11/ex05" -I "$C/c12/ex00" \
       -I "$C/c13/ex00" "$@" "$TMP/main.c" -o "$TMP/p" 2> "$TMP/err"; then
    bad "$name (no compila): $(head -1 "$TMP/err")"
    return
  fi
  local out
  out=$("$TMP/p")
  if [ "$out" = "$exp" ]; then ok "$name"; else bad "$name: [$out] != [$exp]"; fi
}

echo "===================== C 11 (punteros a funcion) ====================="

t "c11/ex00 ft_foreach" "1,2,3," '#include <stdio.h>
void ft_foreach(int *tab, int length, void (*f)(int));
void pr(int n){ printf("%d,", n); }
int main(void){ int a[3]={1,2,3}; ft_foreach(a,3,&pr); ft_foreach(a,0,&pr); return 0; }' \
  "$C/c11/ex00/ft_foreach.c"

t "c11/ex01 ft_map" "1,4,9,16," '#include <stdio.h>
#include <stdlib.h>
int *ft_map(int *tab, int length, int (*f)(int));
int sq(int n){ return n*n; }
int main(void){ int a[4]={1,2,3,4}; int *r=ft_map(a,4,&sq); int i=0;
while(i<4){printf("%d,",r[i]);i++;} free(r); return 0; }' \
  "$C/c11/ex01/ft_map.c"

t "c11/ex02 ft_any" "10" '#include <stdio.h>
int ft_any(char **tab, int (*f)(char*));
int starta(char *s){ return s[0]==97; }
int main(void){ char *a[]={"xy","ab",0}; char *b[]={"xy","zz",0};
printf("%d%d", ft_any(a,&starta), ft_any(b,&starta)); return 0; }' \
  "$C/c11/ex02/ft_any.c"

t "c11/ex03 ft_count_if" "3" '#include <stdio.h>
int ft_count_if(char **tab, int length, int (*f)(char*));
int starta(char *s){ return s[0]==97; }
int main(void){ char *a[]={"ab","ax","zz","aa"};
printf("%d", ft_count_if(a,4,&starta)); return 0; }' \
  "$C/c11/ex03/ft_count_if.c"

t "c11/ex04 ft_is_sort" "110" '#include <stdio.h>
int ft_is_sort(int *tab, int length, int (*f)(int,int));
int cmp(int a,int b){ return a-b; }
int main(void){ int a[]={1,2,3}; int b[]={3,2,1}; int c[]={1,3,2};
printf("%d%d%d", ft_is_sort(a,3,&cmp), ft_is_sort(b,3,&cmp), ft_is_sort(c,3,&cmp));
return 0; }' \
  "$C/c11/ex04/ft_is_sort.c"

# --- ex05 do-op: se compila una vez y se prueban varios juegos de argumentos ---
if cc -Wall -Wextra -Werror -I "$C/c11/ex05" \
     "$C/c11/ex05/do_op.c" "$C/c11/ex05/ops.c" "$C/c11/ex05/ft_atoi.c" \
     "$C/c11/ex05/ft_putnbr.c" -o "$TMP/do-op" 2> "$TMP/err"; then
  dop() { local name=$1 exp=$2; shift 2; local out; out=$("$TMP/do-op" "$@")
    if [ "$out" = "$exp" ]; then ok "$name"; else bad "$name: [$out] != [$exp]"; fi; }
  dop "c11/ex05 do-op 1 + 1" "2" 1 + 1
  dop "c11/ex05 do-op suma sucia" "62" 42amis - --+-20toto12
  dop "c11/ex05 do-op op desconocido p" "0" 1 p 1
  dop "c11/ex05 do-op 1 + toto3" "1" 1 + toto3
  dop "c11/ex05 do-op toto3 + 4" "4" toto3 + 4
  dop "c11/ex05 do-op foo plus bar" "0" foo plus bar
  dop "c11/ex05 do-op division por cero" "Stop : division by zero" 25 / 0
  dop "c11/ex05 do-op modulo por cero" "Stop : modulo by zero" 25 % 0
  dop "c11/ex05 do-op 9 / 2" "4" 9 / 2
  dop "c11/ex05 do-op 9 % 2" "1" 9 % 2
  dop "c11/ex05 do-op 42 * 1" "42" 42 '*' 1
  dop "c11/ex05 do-op sin args" ""
else
  bad "c11/ex05 do-op (no compila): $(head -1 "$TMP/err")"
fi

t "c11/ex06 ft_sort_string_tab" "apple,banana,cherry," '#include <stdio.h>
void ft_sort_string_tab(char **tab);
int main(void){ char *t[]={"banana","apple","cherry",0}; ft_sort_string_tab(t);
int i=0; while(t[i]){printf("%s,",t[i]);i++;} return 0; }' \
  "$C/c11/ex06/ft_sort_string_tab.c"

t "c11/ex07 advanced_sort (asc)" "apple,banana,cherry," '#include <stdio.h>
void ft_advanced_sort_string_tab(char **tab, int(*cmp)(char*,char*));
int c(char*a,char*b){int i=0;while(a[i]&&a[i]==b[i])i++;return a[i]-b[i];}
int main(void){ char *t[]={"banana","apple","cherry",0};
ft_advanced_sort_string_tab(t,&c); int i=0; while(t[i]){printf("%s,",t[i]);i++;}
return 0; }' \
  "$C/c11/ex07/ft_advanced_sort_string_tab.c"

t "c11/ex07 advanced_sort (desc)" "cherry,banana,apple," '#include <stdio.h>
void ft_advanced_sort_string_tab(char **tab, int(*cmp)(char*,char*));
int c(char*a,char*b){int i=0;while(a[i]&&a[i]==b[i])i++;return b[i]-a[i];}
int main(void){ char *t[]={"banana","apple","cherry",0};
ft_advanced_sort_string_tab(t,&c); int i=0; while(t[i]){printf("%s,",t[i]);i++;}
return 0; }' \
  "$C/c11/ex07/ft_advanced_sort_string_tab.c"

echo "===================== C 12 (listas enlazadas) ====================="
CE="$C/c12/ex00/ft_create_elem.c"

t "c12/ex00 ft_create_elem" "42 1" '#include <stdio.h>
#include "ft_list.h"
int main(void){ int x=42; t_list *e=ft_create_elem(&x);
printf("%d %d", *(int*)e->data, e->next==0); return 0; }' \
  "$CE"

t "c12/ex01 push_front" "3,2,1," '#include <stdio.h>
#include "ft_list.h"
void ft_list_push_front(t_list**,void*);
int main(void){ t_list *l=0; static int v[3]={1,2,3}; int i=0;
while(i<3){ft_list_push_front(&l,&v[i]);i++;}
while(l){printf("%d,",*(int*)l->data);l=l->next;} return 0; }' \
  "$C/c12/ex01/ft_list_push_front.c" "$CE"

t "c12/ex02 size" "30" '#include <stdio.h>
#include "ft_list.h"
int ft_list_size(t_list*);
void ft_list_push_front(t_list**,void*);
int main(void){ t_list *l=0; static int v[3]={1,2,3}; int i=0;
while(i<3){ft_list_push_front(&l,&v[i]);i++;}
printf("%d%d", ft_list_size(l), ft_list_size(0)); return 0; }' \
  "$C/c12/ex02/ft_list_size.c" "$C/c12/ex01/ft_list_push_front.c" "$CE"

t "c12/ex03 last" "3 1" '#include <stdio.h>
#include "ft_list.h"
t_list *ft_list_last(t_list*);
static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;
while(i<n){e=ft_create_elem(&a[i]);if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}
int main(void){ int a[3]={1,2,3}; t_list*l=mk(a,3); t_list*last=ft_list_last(l);
printf("%d %d", *(int*)last->data, last->next==0); return 0; }' \
  "$C/c12/ex03/ft_list_last.c" "$CE"

t "c12/ex04 push_back" "1,2,3," '#include <stdio.h>
#include "ft_list.h"
void ft_list_push_back(t_list**,void*);
int main(void){ t_list *l=0; static int v[3]={1,2,3}; int i=0;
while(i<3){ft_list_push_back(&l,&v[i]);i++;}
while(l){printf("%d,",*(int*)l->data);l=l->next;} return 0; }' \
  "$C/c12/ex04/ft_list_push_back.c" "$CE"

t "c12/ex05 push_strs" "c,b,a," '#include <stdio.h>
#include "ft_list.h"
t_list *ft_list_push_strs(int,char**);
int main(void){ char *s[]={"a","b","c"}; t_list*l=ft_list_push_strs(3,s);
while(l){printf("%s,",(char*)l->data);l=l->next;} return 0; }' \
  "$C/c12/ex05/ft_list_push_strs.c" "$CE"

t "c12/ex06 clear" "3" '#include <stdio.h>
#include <stdlib.h>
#include "ft_list.h"
void ft_list_clear(t_list*,void(*)(void*));
int g=0;
void ff(void*p){ g++; free(p); }
static t_list *mkm(int*a,int n){t_list*h=0,*t=0,*e;int i=0;
while(i<n){int*d=malloc(sizeof(int));*d=a[i];e=ft_create_elem(d);
if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}
int main(void){ int a[3]={1,2,3}; t_list*l=mkm(a,3);
ft_list_clear(l,&ff); printf("%d",g); return 0; }' \
  "$C/c12/ex06/ft_list_clear.c" "$CE"

t "c12/ex07 at" "10 30 1" '#include <stdio.h>
#include "ft_list.h"
t_list *ft_list_at(t_list*,unsigned int);
static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;
while(i<n){e=ft_create_elem(&a[i]);if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}
int main(void){ int a[3]={10,20,30}; t_list*l=mk(a,3);
printf("%d %d %d", *(int*)ft_list_at(l,0)->data, *(int*)ft_list_at(l,2)->data,
ft_list_at(l,5)==0); return 0; }' \
  "$C/c12/ex07/ft_list_at.c" "$CE"

t "c12/ex08 reverse" "3,2,1," '#include <stdio.h>
#include "ft_list.h"
void ft_list_reverse(t_list**);
static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;
while(i<n){e=ft_create_elem(&a[i]);if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}
int main(void){ int a[3]={1,2,3}; t_list*l=mk(a,3); ft_list_reverse(&l);
while(l){printf("%d,",*(int*)l->data);l=l->next;} return 0; }' \
  "$C/c12/ex08/ft_list_reverse.c" "$CE"

t "c12/ex09 foreach" "1,2,3," '#include <stdio.h>
#include "ft_list.h"
void ft_list_foreach(t_list*,void(*)(void*));
void pr(void*p){ printf("%d,",*(int*)p); }
static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;
while(i<n){e=ft_create_elem(&a[i]);if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}
int main(void){ int a[3]={1,2,3}; t_list*l=mk(a,3); ft_list_foreach(l,&pr);
return 0; }' \
  "$C/c12/ex09/ft_list_foreach.c" "$CE"

t "c12/ex10 foreach_if" "2,2," '#include <stdio.h>
#include "ft_list.h"
void ft_list_foreach_if(t_list*,void(*)(void*),void*,int(*)(void*,void*));
void pr(void*p){ printf("%d,",*(int*)p); }
int icmp(void*a,void*b){ return *(int*)a-*(int*)b; }
static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;
while(i<n){e=ft_create_elem(&a[i]);if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}
int main(void){ int a[4]={1,2,2,3}; int r=2; t_list*l=mk(a,4);
ft_list_foreach_if(l,&pr,&r,&icmp); return 0; }' \
  "$C/c12/ex10/ft_list_foreach_if.c" "$CE"

t "c12/ex11 find" "21" '#include <stdio.h>
#include "ft_list.h"
t_list *ft_list_find(t_list*,void*,int(*)());
int icmp(void*a,void*b){ return *(int*)a-*(int*)b; }
static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;
while(i<n){e=ft_create_elem(&a[i]);if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}
int main(void){ int a[4]={1,2,3,2}; int r=2; int r9=9; t_list*l=mk(a,4);
printf("%d", *(int*)ft_list_find(l,&r,&icmp)->data);
printf("%d", ft_list_find(l,&r9,&icmp)==0); return 0; }' \
  "$C/c12/ex11/ft_list_find.c" "$CE"

t "c12/ex12 remove_if" "1,3," '#include <stdio.h>
#include <stdlib.h>
#include "ft_list.h"
void ft_list_remove_if(t_list**,void*,int(*)(),void(*)(void*));
int icmp(void*a,void*b){ return *(int*)a-*(int*)b; }
static t_list *mkm(int*a,int n){t_list*h=0,*t=0,*e;int i=0;
while(i<n){int*d=malloc(sizeof(int));*d=a[i];e=ft_create_elem(d);
if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}
int main(void){ int a[5]={1,2,3,2,2}; int r=2; t_list*l=mkm(a,5);
ft_list_remove_if(&l,&r,&icmp,&free);
while(l){printf("%d,",*(int*)l->data);l=l->next;} return 0; }' \
  "$C/c12/ex12/ft_list_remove_if.c" "$CE"

t "c12/ex13 merge" "1,2,3,4,5,6," '#include <stdio.h>
#include "ft_list.h"
void ft_list_merge(t_list**,t_list*);
static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;
while(i<n){e=ft_create_elem(&a[i]);if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}
int main(void){ int x[3]={1,2,3}; int y[3]={4,5,6}; t_list*a=mk(x,3);
t_list*b=mk(y,3); ft_list_merge(&a,b);
while(a){printf("%d,",*(int*)a->data);a=a->next;} return 0; }' \
  "$C/c12/ex13/ft_list_merge.c" "$CE"

t "c12/ex14 sort" "1,2,3," '#include <stdio.h>
#include "ft_list.h"
void ft_list_sort(t_list**,int(*)());
int icmp(void*a,void*b){ return *(int*)a-*(int*)b; }
static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;
while(i<n){e=ft_create_elem(&a[i]);if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}
int main(void){ int a[3]={3,1,2}; t_list*l=mk(a,3); ft_list_sort(&l,&icmp);
while(l){printf("%d,",*(int*)l->data);l=l->next;} return 0; }' \
  "$C/c12/ex14/ft_list_sort.c" "$CE"

t "c12/ex15 reverse_fun" "4,3,2,1," '#include <stdio.h>
#include "ft_list.h"
void ft_list_reverse_fun(t_list*);
static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;
while(i<n){e=ft_create_elem(&a[i]);if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}
int main(void){ int a[4]={1,2,3,4}; t_list*l=mk(a,4); ft_list_reverse_fun(l);
while(l){printf("%d,",*(int*)l->data);l=l->next;} return 0; }' \
  "$C/c12/ex15/ft_list_reverse_fun.c" "$CE"

t "c12/ex16 sorted_insert" "1,2,3," '#include <stdio.h>
#include "ft_list.h"
void ft_sorted_list_insert(t_list**,void*,int(*)());
int icmp(void*a,void*b){ return *(int*)a-*(int*)b; }
int main(void){ t_list*l=0; static int v[3]={3,1,2}; int i=0;
while(i<3){ft_sorted_list_insert(&l,&v[i],&icmp);i++;}
while(l){printf("%d,",*(int*)l->data);l=l->next;} return 0; }' \
  "$C/c12/ex16/ft_sorted_list_insert.c" "$CE"

t "c12/ex17 sorted_merge" "1,2,3,4,5,6," '#include <stdio.h>
#include "ft_list.h"
void ft_sorted_list_merge(t_list**,t_list*,int(*)());
int icmp(void*a,void*b){ return *(int*)a-*(int*)b; }
static t_list *mk(int*a,int n){t_list*h=0,*t=0,*e;int i=0;
while(i<n){e=ft_create_elem(&a[i]);if(!h){h=e;t=e;}else{t->next=e;t=e;}i++;}return h;}
int main(void){ int x[3]={1,3,5}; int y[3]={2,4,6}; t_list*a=mk(x,3);
t_list*b=mk(y,3); ft_sorted_list_merge(&a,b,&icmp);
while(a){printf("%d,",*(int*)a->data);a=a->next;} return 0; }' \
  "$C/c12/ex17/ft_sorted_list_merge.c" "$CE"

echo "===================== C 13 (arboles binarios) ====================="
CN="$C/c13/ex00/btree_create_node.c"
# Arbol de prueba (balanceado, valores 1..7):
#        4
#      /   \
#     2     6
#    / \   / \
#   1   3 5   7
BUILD='static int V[8]={0,1,2,3,4,5,6,7};
t_btree*n1=btree_create_node(&V[1]);
t_btree*n3=btree_create_node(&V[3]);
t_btree*n2=btree_create_node(&V[2]);n2->left=n1;n2->right=n3;
t_btree*n5=btree_create_node(&V[5]);
t_btree*n7=btree_create_node(&V[7]);
t_btree*n6=btree_create_node(&V[6]);n6->left=n5;n6->right=n7;
t_btree*root=btree_create_node(&V[4]);root->left=n2;root->right=n6;'

t "c13/ex00 create_node" "7 1 1" '#include <stdio.h>
#include "ft_btree.h"
int main(void){ int x=7; t_btree*n=btree_create_node(&x);
printf("%d %d %d", *(int*)n->item, n->left==0, n->right==0); return 0; }' \
  "$CN"

t "c13/ex01 apply_prefix" "4,2,1,3,6,5,7," "#include <stdio.h>
#include \"ft_btree.h\"
void btree_apply_prefix(t_btree*,void(*)(void*));
void pr(void*p){ printf(\"%d,\",*(int*)p); }
int main(void){ $BUILD btree_apply_prefix(root,&pr); return 0; }" \
  "$C/c13/ex01/btree_apply_prefix.c" "$CN"

t "c13/ex02 apply_infix" "1,2,3,4,5,6,7," "#include <stdio.h>
#include \"ft_btree.h\"
void btree_apply_infix(t_btree*,void(*)(void*));
void pr(void*p){ printf(\"%d,\",*(int*)p); }
int main(void){ $BUILD btree_apply_infix(root,&pr); return 0; }" \
  "$C/c13/ex02/btree_apply_infix.c" "$CN"

t "c13/ex03 apply_suffix" "1,3,2,5,7,6,4," "#include <stdio.h>
#include \"ft_btree.h\"
void btree_apply_suffix(t_btree*,void(*)(void*));
void pr(void*p){ printf(\"%d,\",*(int*)p); }
int main(void){ $BUILD btree_apply_suffix(root,&pr); return 0; }" \
  "$C/c13/ex03/btree_apply_suffix.c" "$CN"

t "c13/ex04 insert_data" "1,2,3,4,5,7,8,9," '#include <stdio.h>
#include "ft_btree.h"
void btree_insert_data(t_btree**,void*,int(*)(void*,void*));
int icmp(void*a,void*b){ return *(int*)a-*(int*)b; }
void ino(t_btree*r){ if(!r)return; ino(r->left);
printf("%d,",*(int*)r->item); ino(r->right); }
int main(void){ static int v[8]={5,3,8,1,4,7,9,2}; t_btree*root=0; int i=0;
while(i<8){btree_insert_data(&root,&v[i],&icmp);i++;} ino(root); return 0; }' \
  "$C/c13/ex04/btree_insert_data.c" "$CN"

t "c13/ex05 search_item" "5 1" "#include <stdio.h>
#include \"ft_btree.h\"
void *btree_search_item(t_btree*,void*,int(*)(void*,void*));
int icmp(void*a,void*b){ return *(int*)a-*(int*)b; }
int main(void){ $BUILD int k=5; int m=99;
printf(\"%d %d\", *(int*)btree_search_item(root,&k,&icmp),
btree_search_item(root,&m,&icmp)==0); return 0; }" \
  "$C/c13/ex05/btree_search_item.c" "$CN"

t "c13/ex06 level_count" "310" "#include <stdio.h>
#include \"ft_btree.h\"
int btree_level_count(t_btree*);
int main(void){ $BUILD int s=1; t_btree*one=btree_create_node(&s);
printf(\"%d%d%d\", btree_level_count(root), btree_level_count(one),
btree_level_count(0)); return 0; }" \
  "$C/c13/ex06/btree_level_count.c" "$CN"

t "c13/ex07 apply_by_level" "4:0:1 2:1:1 6:1:0 1:2:1 3:2:0 5:2:0 7:2:0 " \
  "#include <stdio.h>
#include \"ft_btree.h\"
void btree_apply_by_level(t_btree*,void(*)(void*,int,int));
void ap(void*it,int lvl,int f){ printf(\"%d:%d:%d \", *(int*)it, lvl, f); }
int main(void){ $BUILD btree_apply_by_level(root,&ap); return 0; }" \
  "$C/c13/ex07/btree_apply_by_level.c" "$CN"

echo
echo "==== RESULTADO C11-C13: $PASS PASS / $FAIL FAIL ===="
rm -rf "$TMP"
[ "$FAIL" -eq 0 ]
