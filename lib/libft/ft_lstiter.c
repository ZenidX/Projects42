/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstiter.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/04 01:20:59 by xalara            #+#    #+#             */
/*   Updated: 2026/10/04 04:42:40 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_lstiter(t_list *lst, void (*f)(void *))
{
	t_list	*n;

	n = lst;
	if (lst && f)
	{
		while (n->next != NULL)
		{
			(*f)(n->content);
			n = n->next;
		}
		(*f)(n->content);
	}
}
