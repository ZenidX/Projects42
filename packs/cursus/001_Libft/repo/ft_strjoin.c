/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strjoin.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/02 02:25:51 by xalara            #+#    #+#             */
/*   Updated: 2026/10/03 20:19:31 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strjoin(char const *s1, char const *s2)
{
	char	*p;
	size_t	l1;
	size_t	l2;
	size_t	i;

	l1 = ft_strlen(s1);
	l2 = ft_strlen(s2);
	p = (char *)malloc(sizeof(char) * (l1 + l2 + 1));
	i = 0;
	while (i < l1)
	{
		p[i] = s1[i];
		i++;
	}
	i = 0;
	while (i < l2)
	{
		p[l1 + i] = s2[i];
		i++;
	}
	return (p);
}
