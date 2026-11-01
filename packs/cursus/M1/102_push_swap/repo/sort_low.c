/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort_low.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/01 13:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/11/01 13:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** Metodo de adaptive para poco desorden (d < 0.2): casi todo esta ya en su
** sitio, asi que se deja quieto lo que esta bien y solo se recolocan los
** pocos que no. "Lo que esta bien" es la subsecuencia creciente mas larga
** (LIS) de a: esos elementos ya estan en orden entre si.
*/

/*
** len[i] = largo de la subsecuencia creciente mas larga que acaba en i,
** prev[i] = el elemento anterior en esa subsecuencia (-1 si no hay).
** Es O(n^2) en tiempo de calculo, pero no mueve las pilas (lo que puntua
** son las operaciones).
*/
static void	lis_lengths(int *vals, int n, int *len, int *prev)
{
	int	i;
	int	j;

	i = 0;
	while (i < n)
	{
		len[i] = 1;
		prev[i] = -1;
		j = 0;
		while (j < i)
		{
			if (vals[j] < vals[i] && len[j] + 1 > len[i])
			{
				len[i] = len[j] + 1;
				prev[i] = j;
			}
			j++;
		}
		i++;
	}
}

/*
** Array keep: keep[i] = 1 si el elemento en la posicion i esta en la LIS.
** Se parte del final mas largo y se va hacia atras con prev. len y prev van
** en un solo malloc (prev es la segunda mitad) y la primera mitad se
** reutiliza como keep. NULL si falla malloc.
*/
static int	*lis_keep(int *vals, int n)
{
	int	*len;
	int	*prev;
	int	best;
	int	i;

	len = malloc(sizeof(int) * n * 2);
	if (!len)
		return (NULL);
	prev = len + n;
	lis_lengths(vals, n, len, prev);
	best = 0;
	i = 0;
	while (++i < n)
		if (len[i] > len[best])
			best = i;
	ft_bzero(len, sizeof(int) * n);
	while (best >= 0)
	{
		len[best] = 1;
		best = prev[best];
	}
	return (len);
}

/*
** Recorre a desde la cima: los de la LIS se quedan (ra) y el resto va a b
** (pb). Primero cuenta cuantos no son de la LIS (left) para parar en cuanto
** ha pasado el ultimo, sin dar la vuelta entera.
*/
static void	push_non_lis(t_ps *ps, int *keep)
{
	int	left;
	int	i;

	left = 0;
	i = 0;
	while (i < ps->size)
		left += !keep[i++];
	i = 0;
	while (left > 0)
	{
		if (keep[i++])
			ra(ps);
		else
		{
			pb(ps);
			left--;
		}
	}
}

/*
** Donde hay que meter v en a (que esta ordenada, quiza rotada): encima del
** menor que sea mayor que v. Si no hay ninguno mayor, encima del menor de
** todos. Las dos cosas a la vez: a los <= v se les suma INT_MAX para que
** solo ganen si no hay ninguno mayor, y se busca el minimo.
*/
static int	insert_pos(t_list *a, int v)
{
	long	key;
	long	best;
	int		pos;
	int		i;

	best = LONG_MAX;
	pos = 0;
	i = 0;
	while (a)
	{
		key = *(int *)a->content;
		if (key < v)
			key += INT_MAX;
		if (key < best)
		{
			best = key;
			pos = i;
		}
		a = a->next;
		i++;
	}
	return (pos);
}

/*
** 1. Calcula la LIS sobre una copia de a.
** 2. Manda a b los que no son de la LIS: a queda ordenada (rotada).
** 3. Devuelve cada uno de b a su hueco de a (rotando por el camino corto).
** 4. Sube el 0 a la cima.
** Si falla malloc se ordena con sort_medium, que no lo necesita.
**
** Operaciones: el paso 2 cuesta como mucho n, y con poco desorden quedan
** fuera de la LIS pocos (k), que ademas se insertan cerca de donde se
** sacaron: O(n + k * n), O(n) cuando k es pequeno.
*/
void	sort_low(t_ps *ps)
{
	int	*vals;
	int	*keep;

	vals = copy_values(ps);
	keep = NULL;
	if (vals)
		keep = lis_keep(vals, ps->size);
	free(vals);
	if (!keep)
	{
		sort_medium(ps);
		return ;
	}
	push_non_lis(ps, keep);
	free(keep);
	while (ps->b)
	{
		rotate_to_top_a(ps, insert_pos(ps->a, *(int *)ps->b->content));
		pa(ps);
	}
	rotate_to_top_a(ps, stack_pos(ps->a, 0));
}
