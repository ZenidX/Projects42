/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   rotate_reverse.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/24 21:04:53 by xalara            #+#    #+#             */
/*   Updated: 2026/10/25 01:38:51 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** Lo contrario de rotate: el fondo pasa a la cima.
**     [1]->[2]->[3]  =>  [3]->[1]->[2]
** Hay que encontrar el penultimo (pen: el que cumple pen->next->next ==
** NULL), porque su next pasa a ser NULL; el ultimo se engancha delante de
** la cima y se convierte en la nueva cima.
*/
static void	reverse_rotate(t_list **s)
{
	t_list	*pen;
	t_list	*last;

	if (!*s || !(*s)->next)
		return ;
	pen = *s;
	while (pen->next->next)
		pen = pen->next;
	last = pen->next;
	pen->next = NULL;
	last->next = *s;
	*s = last;
}

void	rra(t_ps *ps)
{
	reverse_rotate(&ps->a);
	ps->count[RRA]++;
	ft_putendl_fd("rra", 1);
}

void	rrb(t_ps *ps)
{
	reverse_rotate(&ps->b);
	ps->count[RRB]++;
	ft_putendl_fd("rrb", 1);
}

void	rrr(t_ps *ps)
{
	reverse_rotate(&ps->a);
	reverse_rotate(&ps->b);
	ps->count[RRR]++;
	ft_putendl_fd("rrr", 1);
}
