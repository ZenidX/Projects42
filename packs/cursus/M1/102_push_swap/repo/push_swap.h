/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   push_swap.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/25 01:06:57 by xalara            #+#    #+#             */
/*   Updated: 2026/10/30 12:48:57 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PUSH_SWAP_H
# define PUSH_SWAP_H

# include <limits.h>
# include <stdlib.h>
# include <unistd.h>
# include "libft.h"

/*
** Estrategia de ordenacion. ADAPTIVE va primero para que valga 0: asi
** ft_bzero sobre t_ps ya la deja como la de por defecto. Las otras tres se
** fuerzan con --simple, --medium y --complex.
*/
typedef enum e_strategy
{
	ADAPTIVE,
	SIMPLE,
	MEDIUM,
	COMPLEX
}	t_strategy;

/*
** Las 11 operaciones del enunciado. Cada valor es el indice de su contador
** en t_ps.count (count[RA] = cuantas veces se ha hecho ra), que es lo que
** luego imprime --bench.
*/
typedef enum e_op
{
	SA,
	SB,
	SS,
	PA,
	PB,
	RA,
	RB,
	RR,
	RRA,
	RRB,
	RRR
}	t_op;

/*
** Todo el estado del programa; se pasa por puntero a todas las funciones.
**
**   a, b          las dos pilas. Son listas de libft; el primer nodo es la
**                 cima y cada content es un int* (malloc de un int). Tras
**                 normalize() esos int son rangos 0..size-1, no los numeros
**                 originales.
**   size          cuantos numeros se han leido (los de a al empezar).
**   mistakes      pares (i<j) con a[i] > a[j]: los que estan al reves.
**   pairs         pares (i<j) totales: size * (size - 1) / 2.
**                 desorden = mistakes / pairs, entre 0 (ordenado) y 1.
**   strategy      la estrategia que se va a ejecutar.
**   strategy_set  1 si ya se paso un flag de estrategia (para dar Error si
**                 llega otro).
**   bench         1 si se paso --bench.
**   count         un contador por operacion, indexado con t_op.
*/
typedef struct s_ps
{
	t_list		*a;
	t_list		*b;
	int			size;
	long		mistakes;
	long		pairs;
	t_strategy	strategy;
	int			strategy_set;
	int			bench;
	int			count[11];
}	t_ps;

/* parse.c, index.c, bench.c: de los argumentos a una pila lista y medida */
int		parse_args(t_ps *ps, int n, char **args);
int		normalize(t_ps *ps);
void	compute_disorder(t_ps *ps);
int		disorder_regime(t_ps *ps);
void	print_bench(t_ps *ps);

/* push_swap.c y sort_*.c: los algoritmos y quien elige cual se ejecuta */
void	run_strategy(t_ps *ps);
void	sort_simple(t_ps *ps);
void	sort_medium(t_ps *ps);
void	sort_complex(t_ps *ps);
void	sort_adaptive(t_ps *ps);

/* push_swap.c: liberar memoria y salir con error */
void	free_split(char **words);
void	free_ps(t_ps *ps);
void	error_exit(t_ps *ps);

/*
** swap.c, push.c, rotate.c, rotate_reverse.c: las 11 operaciones. Cada una
** mueve las pilas, suma su contador y escribe su nombre por stdout. Los
** algoritmos solo deben mover las pilas a traves de ellas.
*/
void	sa(t_ps *ps);
void	sb(t_ps *ps);
void	ss(t_ps *ps);
void	pa(t_ps *ps);
void	pb(t_ps *ps);
void	ra(t_ps *ps);
void	rb(t_ps *ps);
void	rr(t_ps *ps);
void	rra(t_ps *ps);
void	rrb(t_ps *ps);
void	rrr(t_ps *ps);

#endif
