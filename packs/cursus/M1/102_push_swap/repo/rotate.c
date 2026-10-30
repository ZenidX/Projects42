/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   rotate.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/24 21:04:53 by xalara            #+#    #+#             */
/*   Updated: 2026/10/25 01:38:51 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** La cima pasa al fondo y todos suben un puesto:
**     [1]->[2]->[3]  =>  [2]->[3]->[1]
** Se guarda el segundo (la nueva cima), se engancha el primero detras del
** ultimo y se corta su next. ft_lstlast recorre la lista entera; da igual,
** porque solo se puntua el numero de operaciones.
*/
static void	rotate(t_list **s)
{
	t_list	*tmp;

	if (!*s || !(*s)->next)
		return ;
	tmp = (*s)->next;
	ft_lstlast(*s)->next = *s;
	(*s)->next = NULL;
	*s = tmp;
}

void	ra(t_ps *ps)
{
	rotate(&ps->a);
	ps->count[RA]++;
	ft_putendl_fd("ra", 1);
}

void	rb(t_ps *ps)
{
	rotate(&ps->b);
	ps->count[RB]++;
	ft_putendl_fd("rb", 1);
}

void	rr(t_ps *ps)
{
	rotate(&ps->a);
	rotate(&ps->b);
	ps->count[RR]++;
	ft_putendl_fd("rr", 1);
}
