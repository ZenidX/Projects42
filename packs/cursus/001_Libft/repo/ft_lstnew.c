/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstnew.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/04 00:16:08 by xalara            #+#    #+#             */
/*   Updated: 2026/10/04 00:20:29 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

t_list	*ft_lstnew(void *content)
{
	t_list *n;

	n = (t_list *)malloc(sizeof(t_list));
	if (n == NULL)
		return (NULL);
	else
	{
		n->content = content;
		n->next = NULL;
		return (n);
	}
}
