/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strjoin.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.co      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 01:49:55 by xalara            #+#    #+#             */
/*   Updated: 2026/08/04 02:08:43 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

int	ft_strlen(char *str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		i++;
	}
	return (i);
}

int	ft_strcpy(char *dest, char *src)
{
	int	i;

	i = 0;
	while (src[i])
	{
		dest[i] = src[i];
		i++;
	}
	return (i);
}

char	*ft_strjoin(int size, char **strs, char *sep)
{
	int	i;
	int	c;
	int	l_sep;
	char	*r;

	l_sep = ft_strlen(sep);
	c = 0;
	i = 0;
	while (i < size)
	{
		c += ft_strlen(strs[i]);
		c += l_sep;
		i++;
	}
	c -= l_sep;
	r = malloc(sizeof(char) * c);
	c = 0;
	i = 0;
	while (i < size - 1)
	{
		c += ft_strcpy(&r[c], strs[i]);
		c += ft_strcpy(&r[c], sep);
		i++;
	}
	r = ft_strcpy(r,
}
