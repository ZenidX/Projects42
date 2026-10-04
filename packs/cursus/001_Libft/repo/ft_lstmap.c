/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstmap.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/04 01:24:14 by xalara            #+#    #+#             */
/*   Updated: 2026/10/04 04:57:14 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

t_list	*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *))
{
	t_list	*r;
	t_list	*n;
	void	*content;

	if (!lst || !f)
		return (NULL);
	r = NULL;
	while (lst)
	{
		content = lst->content;
		n = ft_lstnew(f(content));
		if (!n)
		{
			if (del)
				del(content);
				ft_lstclear(&r, del);
			return (NULL);
		}
		ft_lstadd_back(&r, n);
		lst = lst->next;
	}
	return (r);
}
