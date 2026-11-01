/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort_complex.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/25 01:38:51 by xalara            #+#    #+#             */
/*   Updated: 2026/11/01 13:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** Estrategia --complex, O(n log n): radix binario (LSD) sobre los rangos.
** En cada bit, empezando por el de menos peso, se recorre a entera: los que
** tienen ese bit a 0 van a b (pb) y los que lo tienen a 1 se quedan (ra).
** Despues pa de todo. Es estable, asi que tras el ultimo bit a queda
** ordenada.
**
** Hay tantas pasadas como bits tiene size - 1 (log2(n)) y cada una cuesta
** como mucho 2n operaciones: O(n log n).
*/
void	sort_complex(t_ps *ps)
{
	int	bit;
	int	i;

	bit = 0;
	while ((ps->size - 1) >> bit)
	{
		i = 0;
		while (i < ps->size)
		{
			if ((*(int *)ps->a->content >> bit) & 1)
				ra(ps);
			else
				pb(ps);
			i++;
		}
		while (ps->b)
			pa(ps);
		bit++;
	}
}
