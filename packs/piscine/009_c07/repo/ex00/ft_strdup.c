/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strdup.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/30 19:32:50 by xalara            #+#    #+#             */
/*   Updated: 2026/08/05 16:38:18 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>

unsigned int	ft_strlen(char *str)
{
	unsigned int	i;

	i = 0;
	while (str[i])
		i++;
	return (i);
}

void	ft_strncpy(char *d, char *s, unsigned int n)
{
	unsigned int	i;

	i = 0;
	while (s[i] && i < n)
	{
		d[i] = s[i];
		i++;
	}
	d[i] = '\0';
}

char	*ft_strdup(char *src)
{
	char				*p;
	unsigned int		i;

	i = ft_strlen(src);
	p = (char *)malloc(sizeof(char) * (i + 1));
	if (!p)
		return (NULL);
	else
	{
		ft_strncpy(p, src, i + 1);
		return (p);
	}
}
