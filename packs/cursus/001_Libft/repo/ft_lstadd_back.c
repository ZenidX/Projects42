/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstadd_back.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/04 00:32:33 by xalara            #+#    #+#             */
/*   Updated: 2026/10/04 00:58:11 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void ft_lstadd_back(t_list **lst, t_list *new)
{
	t_list	*n;

	if(lst == NULL || *lst == NULL)
	{
		n = NULL;
		*lst = new;
	}
	else
	{
		n = *lst;
		while (n->next != NULL)
			n = n->next;
		n->next = new;
	}
}
