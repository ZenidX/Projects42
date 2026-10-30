/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strdup.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/02 01:45:06 by xalara            #+#    #+#             */
/*   Updated: 2026/10/04 15:19:30 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strdup(const char *s)
{
	char	*p;
	size_t	l;
	size_t	i;

	l = ft_strlen(s);
	p = malloc(sizeof(char) * (l + 1));
	if (!p)
		return (NULL);
	i = 0;
	while (i < l)
	{
		p[i] = s[i];
		i++;
	}
	p[i] = '\0';
	return (p);
}
