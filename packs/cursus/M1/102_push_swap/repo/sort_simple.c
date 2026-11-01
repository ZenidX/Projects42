/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort_simple.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/25 01:38:51 by xalara            #+#    #+#             */
/*   Updated: 2026/11/01 13:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/* 1 si la pila ya esta ordenada de menor (cima) a mayor. */
static int	is_sorted(t_list *s)
{
	while (s && s->next)
	{
		if (*(int *)s->content > *(int *)s->next->content)
			return (0);
		s = s->next;
	}
	return (1);
}

/*
** Estrategia --simple, O(n^2): seleccion. Como a esta normalizada, el menor
** que queda en a es siempre el siguiente rango (0, 1, 2...): se busca su
** posicion, se sube a la cima por el camino mas corto (ra o rra) y se pasa
** a b. En cuanto lo que queda en a ya esta ordenado se para. Al final b
** tiene los menores con el mayor arriba, asi que basta con pa hasta
** vaciarla.
**
** Cada busqueda + rotacion cuesta hasta n/2, y se hace n veces: O(n^2).
*/
void	sort_simple(t_ps *ps)
{
	int	next;

	next = 0;
	while (ps->a && !is_sorted(ps->a))
	{
		rotate_to_top_a(ps, stack_pos(ps->a, next));
		pb(ps);
		next++;
	}
	while (ps->b)
		pa(ps);
}
