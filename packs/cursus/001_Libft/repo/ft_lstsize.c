/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstsize.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/04 00:25:14 by xalara            #+#    #+#             */
/*   Updated: 2026/10/04 00:28:55 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

unsigned int	ft_lstsize(t_list *lst)
{
	t_list 			*n;
	unsigned int	i;

	n = lst;
	if (lst == NULL)
		return (0);
	i = 1;
	while (n->next !=NULL)
	{
		n = n->next;
		i++;
	}
	return (i);
}
