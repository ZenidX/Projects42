/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort_medium.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/25 01:38:51 by xalara            #+#    #+#             */
/*   Updated: 2026/11/01 13:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/* Raiz cuadrada entera: el mayor r con r * r <= n. */
static int	isqrt(int n)
{
	int	r;

	r = 1;
	while ((r + 1) * (r + 1) <= n)
		r++;
	return (r);
}

/*
** Pasa todo a b por tramos. i cuenta cuantos van ya en b; se acepta la cima
** de a si su rango es <= i + range (esta en el tramo actual):
**   - si ademas es <= i, es de los pequenos del tramo: pb y rb, abajo de b
**   - si no, pb y se queda arriba
**   - si no esta en el tramo, ra y se mira el siguiente
** Siempre hay alguno en el tramo: el menor que queda en a es <= i. Asi b
** queda como una "mariposa": los grandes cerca de la cima, los pequenos
** abajo, y sacar el mayor cada vez es barato.
*/
static void	push_chunks(t_ps *ps, int range)
{
	int	i;
	int	v;

	i = 0;
	while (ps->a)
	{
		v = *(int *)ps->a->content;
		if (v <= i + range)
		{
			pb(ps);
			if (v <= i)
				rb(ps);
			i++;
		}
		else
			ra(ps);
	}
}

/*
** Estrategia --medium, O(n sqrt n): chunks. Con tramos de unos 1.5 sqrt(n)
** rangos, cada elemento espera como mucho un tramo de rotaciones para
** entrar en b, y luego se devuelven a a de mayor a menor: el mayor que
** queda en b es su tamano - 1 (b tiene justo los rangos 0..tamano-1), se
** sube por el camino mas corto y pa.
*/
void	sort_medium(t_ps *ps)
{
	push_chunks(ps, isqrt(ps->size) * 3 / 2);
	while (ps->b)
	{
		rotate_to_top_b(ps, stack_pos(ps->b, ft_lstsize(ps->b) - 1));
		pa(ps);
	}
}
