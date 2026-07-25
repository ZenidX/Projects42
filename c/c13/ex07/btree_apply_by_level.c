/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   btree_apply_by_level.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 12:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/07/23 12:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include "ft_btree.h"

static t_queue	*new_qnode(t_btree *node, int level)
{
	t_queue	*q;

	q = malloc(sizeof(t_queue));
	if (!q)
		return (NULL);
	q->node = node;
	q->level = level;
	q->next = NULL;
	return (q);
}

static t_queue	*enqueue_kids(t_queue *tail, t_btree *node, int level)
{
	t_queue	*q;

	if (node->left)
	{
		q = new_qnode(node->left, level + 1);
		if (!q)
			return (tail);
		tail->next = q;
		tail = q;
	}
	if (node->right)
	{
		q = new_qnode(node->right, level + 1);
		if (!q)
			return (tail);
		tail->next = q;
		tail = q;
	}
	return (tail);
}

void	btree_apply_by_level(t_btree *root, void (*applyf)(void *, int, int))
{
	t_queue	*head;
	t_queue	*tail;
	t_queue	*tmp;
	int		prev;

	if (!root)
		return ;
	head = new_qnode(root, 0);
	tail = head;
	prev = -1;
	while (head)
	{
		applyf(head->node->item, head->level, head->level != prev);
		prev = head->level;
		tail = enqueue_kids(tail, head->node, head->level);
		tmp = head;
		head = head->next;
		free(tmp);
	}
}
