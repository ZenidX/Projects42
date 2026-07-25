/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_list_reverse_fun.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 12:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/07/23 12:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_list.h"

static int	list_len(t_list *l)
{
	int	n;

	n = 0;
	while (l)
	{
		n++;
		l = l->next;
	}
	return (n);
}

static t_list	*node_at(t_list *begin, int pos)
{
	int	i;

	i = 0;
	while (i < pos)
	{
		begin = begin->next;
		i++;
	}
	return (begin);
}

void	ft_list_reverse_fun(t_list *begin_list)
{
	t_list	*left;
	t_list	*right;
	void	*tmp;
	int		i;
	int		n;

	n = list_len(begin_list);
	left = begin_list;
	i = 0;
	while (i < n / 2)
	{
		right = node_at(begin_list, n - 1 - i);
		tmp = left->data;
		left->data = right->data;
		right->data = tmp;
		left = left->next;
		i++;
	}
}
