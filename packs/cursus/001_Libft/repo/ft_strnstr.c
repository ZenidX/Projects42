/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strnstr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/02 00:24:21 by xalara            #+#    #+#             */
/*   Updated: 2026/10/02 01:03:42 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strnstr(const char *big, const char *little, size_t len)
{
	size_t	i;
	size_t	j;
	char	*p;
	char	*q;

	p = (char *) big;
	q = (char *) little;
	i = 0;
	while (p[i])
	{
		j = 0;
		while (p[i + j] == q[j] && p[i + j] && q[j] && i + j < len)
			j++;
		if (!(q[j]))
			return (&p[i]);
		i++;
	}
	return (NULL);
}
