/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   stack_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/31 03:16:01 by xalara            #+#    #+#             */
/*   Updated: 2026/11/01 13:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** Auxiliares de pila que comparten las estrategias. Las dos traducen entre
** nodo y posicion, contando desde la cima (posicion 0):
**     lstindex   nodo     -> posicion
**     indexlst   posicion -> nodo
*/

/*
** Posicion del nodo p dentro de list: 0 si es la cima, 1 si es el segundo...
** Se compara el nodo (el puntero), no su content. Si p no esta en la
** lista devuelve -1.
**     list [a]->[b]->[c],  p = [c]  ->  2
*/
int	lstindex(t_list *list, t_list *p)
{
	int		i;

	if (list == p)
		return (0);
	i = 1;
	while (list->next && list->next != p)
	{
		list = list->next;
		i++;
	}
	if (!(list->next))
		return (-1);
	return (i);
}

/*
** Lo contrario de lstindex: el nodo que esta en la posicion index, con la
** cima en la posicion 0. Avanza index veces por ->next.
**     list [a]->[b]->[c],  index = 1  ->  [b]
*/
t_list	*indexlst(t_list *list, size_t index)
{
	t_list	*r;

	r = list;
	while (list && index > 0)
	{
		list = list->next;
		index--;
	}
	r = list;
	return (r);
}

/*
** Posicion del valor v en la pila (0 = cima), o -1 si no esta. Como las
** pilas tienen rangos, buscar "el 3" es buscar el cuarto menor.
*/
int	stack_pos(t_list *s, int v)
{
	int	i;

	i = 0;
	while (s)
	{
		if (*(int *)s->content == v)
			return (i);
		s = s->next;
		i++;
	}
	return (-1);
}

/*
** Sube a la cima de a el nodo de la posicion pos por el camino mas corto:
** ra pos veces si esta en la mitad de arriba, rra (size - pos) veces si
** esta en la de abajo.
*/
void	rotate_to_top_a(t_ps *ps, int pos)
{
	int	size;

	size = ft_lstsize(ps->a);
	if (pos <= size / 2)
		while (pos-- > 0)
			ra(ps);
	else
		while (pos++ < size)
			rra(ps);
}

/* Lo mismo para b, con rb y rrb. */
void	rotate_to_top_b(t_ps *ps, int pos)
{
	int	size;

	size = ft_lstsize(ps->b);
	if (pos <= size / 2)
		while (pos-- > 0)
			rb(ps);
	else
		while (pos++ < size)
			rrb(ps);
}
