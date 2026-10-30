/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   index.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/25 01:38:51 by xalara            #+#    #+#             */
/*   Updated: 2026/10/25 01:38:51 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** Normalizacion y medida del desorden.
**
** normalize() cambia cada numero por su rango: cuantos numeros hay menores
** que el. Los algoritmos trabajan siempre con 0..size-1 y no con los
** valores originales:
**
**     entrada   42   -7   100   5
**     rangos     2    0     3   1
**
** El orden relativo no cambia, asi que ordenar los rangos es ordenar los
** numeros, y el desorden sale igual. A cambio, radix puede leer los bits
** directamente (todos >= 0 y pequenos) y los chunks saben que "los k
** menores" son los valores 0..k-1.
*/

/*
** Copia la pila a un array. Hace falta porque normalize() va pisando los
** content con los rangos y cada rango se tiene que calcular con los
** valores originales.
*/
static int	*copy_values(t_ps *ps)
{
	int		*vals;
	t_list	*n;
	int		i;

	vals = malloc(sizeof(int) * ps->size);
	if (!vals)
		return (NULL);
	n = ps->a;
	i = 0;
	while (n)
	{
		vals[i++] = *(int *)n->content;
		n = n->next;
	}
	return (vals);
}

/*
** Rango de vals[i] = cuantos valores son menores que el. Si encuentra otro
** igual (en otra posicion) devuelve -1: es un duplicado y eso es Error.
*/
static int	rank_of(int *vals, int size, int i)
{
	int	rank;
	int	j;

	rank = 0;
	j = 0;
	while (j < size)
	{
		if (j != i && vals[j] == vals[i])
			return (-1);
		if (vals[j] < vals[i])
			rank++;
		j++;
	}
	return (rank);
}

/*
** Sustituye en la propia pila cada valor por su rango. O(n^2), sin
** problema para los tamanos de push_swap (aqui solo se puntuan las
** operaciones, no el tiempo). Devuelve 0 si falla malloc o hay duplicados.
*/
int	normalize(t_ps *ps)
{
	int		*vals;
	t_list	*n;
	int		i;
	int		rank;

	vals = copy_values(ps);
	if (!vals)
		return (0);
	n = ps->a;
	i = 0;
	while (n)
	{
		rank = rank_of(vals, ps->size, i++);
		if (rank < 0)
		{
			free(vals);
			return (0);
		}
		*(int *)n->content = rank;
		n = n->next;
	}
	free(vals);
	return (1);
}

/*
** El pseudocodigo del enunciado: para cada par (i, j) con i antes que j,
** suma uno a pairs, y si a[i] > a[j] (estan al reves) suma uno a mistakes.
** desorden = mistakes / pairs. No se divide aqui para no usar float:
** disorder_regime y bench trabajan con los dos enteros.
**
**     3 2 1      pairs 3,  mistakes 3  -> 100 %
**     1 3 2      pairs 3,  mistakes 1  ->  33.33 %
**     1 2 3 5 4  pairs 10, mistakes 1  ->  10 %
*/
void	compute_disorder(t_ps *ps)
{
	t_list	*i;
	t_list	*j;

	ps->mistakes = 0;
	ps->pairs = 0;
	i = ps->a;
	while (i)
	{
		j = i->next;
		while (j)
		{
			ps->pairs++;
			if (*(int *)i->content > *(int *)j->content)
				ps->mistakes++;
			j = j->next;
		}
		i = i->next;
	}
}

/*
** Que regimen pide el enunciado segun el desorden d = mistakes / pairs,
** multiplicando en vez de dividir para no usar float:
**
**     mistakes * 5 < pairs   <=>  d < 0.2    -> 0  (metodo O(n))
**     mistakes * 2 < pairs   <=>  d < 0.5    -> 1  (metodo O(n sqrt n))
**     si no                       d >= 0.5   -> 2  (metodo O(n log n))
*/
int	disorder_regime(t_ps *ps)
{
	if (ps->mistakes * 5 < ps->pairs)
		return (0);
	if (ps->mistakes * 2 < ps->pairs)
		return (1);
	return (2);
}
