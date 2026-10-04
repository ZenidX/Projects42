/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstclear.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/04 01:15:19 by xalara            #+#    #+#             */
/*   Updated: 2026/10/04 01:20:33 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_lstclear(t_list **lst, void (*del)(void *))
{
	if ((*lst)->next != NULL)
	{
		ft_lstclear(&((*lst)->next),del);
	}
	(*del)((*lst)->content);
	free(*lst);
	*lst = NULL;
}
