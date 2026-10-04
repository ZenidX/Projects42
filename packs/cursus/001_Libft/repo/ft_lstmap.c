/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_lstmap.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/04 01:24:14 by xalara            #+#    #+#             */
/*   Updated: 2026/10/04 02:12:02 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

t_list	*ft_lstmap(t_list *lst, void *(*f)(void *), void (*del)(void *))
{
	t_list	*r;
	t_list	*n;
	t_list	*s;

	if (!lst || !f || !del)
		return (NULL);
	r = NULL;
	while (lst)
	{
		n = ft_lstnew(f(lst->content));
		if(!n)
		{
			while (!r)
			{
				s = r->next;
				del(r->content);
				free(r);
				r = s;
			}
			return (NULL);
		}
		ft_lstadd_back(&r, n);
		lst = lst->next;
	}
	return (r);
}
