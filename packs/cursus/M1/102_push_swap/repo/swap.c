/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   swap.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/24 20:15:45 by xalara            #+#    #+#             */
/*   Updated: 2026/10/25 01:38:51 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** Mismo patron en los cuatro ficheros de operaciones: una funcion static
** que solo mueve punteros y, encima, las publicas (sa, sb, ss...) que
** llaman a esa, suman su contador y escriben su nombre por stdout. Las
** dobles (ss, rr, rrr) mueven las dos pilas pero cuentan y escriben UNA
** operacion: por eso salen mas baratas que hacer las dos por separado.
** Con una pila de 0 o 1 nodos la operacion no hace nada, pero se escribe y
** se cuenta igual.
*/

/*
** Intercambia los dos primeros. No se mueven los nodos: se intercambian
** sus content (los int*), que es lo mismo y mas sencillo.
**     [1]->[2]->[3]  =>  [2]->[1]->[3]
*/
static void	swap(t_list *s)
{
	void	*tmp;

	if (!s || !s->next)
		return ;
	tmp = s->content;
	s->content = s->next->content;
	s->next->content = tmp;
}

void	sa(t_ps *ps)
{
	swap(ps->a);
	ps->count[SA]++;
	ft_putendl_fd("sa", 1);
}

void	sb(t_ps *ps)
{
	swap(ps->b);
	ps->count[SB]++;
	ft_putendl_fd("sb", 1);
}

void	ss(t_ps *ps)
{
	swap(ps->a);
	swap(ps->b);
	ps->count[SS]++;
	ft_putendl_fd("ss", 1);
}
