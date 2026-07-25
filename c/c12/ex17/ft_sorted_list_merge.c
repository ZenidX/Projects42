/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_sorted_list_merge.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 12:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/07/23 12:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_list.h"

static void	ins_sorted(t_list **head, t_list *node, int (*cmp)())
{
	t_list	*cur;

	if (!*head || cmp((*head)->data, node->data) > 0)
	{
		node->next = *head;
		*head = node;
		return ;
	}
	cur = *head;
	while (cur->next && cmp(cur->next->data, node->data) <= 0)
		cur = cur->next;
	node->next = cur->next;
	cur->next = node;
}

void	ft_sorted_list_merge(t_list **begin_list1, t_list *begin_list2,
		int (*cmp)())
{
	t_list	*next2;

	while (begin_list2)
	{
		next2 = begin_list2->next;
		ins_sorted(begin_list1, begin_list2, cmp);
		begin_list2 = next2;
	}
}
