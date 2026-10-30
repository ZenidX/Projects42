/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   push.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: gabgalla <gabgalla@student.42barcelon      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/24 20:26:28 by gabgalla          #+#    #+#             */
/*   Updated: 2026/10/25 01:38:51 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** Saca la cima de src y la pone como cima de dest, moviendo el nodo
** entero (sin malloc ni free):
**     src  [x]->[y]->...        src  [y]->...
**     dest [z]->...        =>   dest [x]->[z]->...
** pa: de b a a.  pb: de a a b.  Si src esta vacia no hace nada.
*/
static void	push(t_list **dest, t_list **src)
{
	t_list	*tmp;

	if (!*src)
		return ;
	tmp = *src;
	*src = (*src)->next;
	tmp->next = *dest;
	*dest = tmp;
}

void	pa(t_ps *ps)
{
	push(&ps->a, &ps->b);
	ps->count[PA]++;
	ft_putendl_fd("pa", 1);
}

void	pb(t_ps *ps)
{
	push(&ps->b, &ps->a);
	ps->count[PB]++;
	ft_putendl_fd("pb", 1);
}
