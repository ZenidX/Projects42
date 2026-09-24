/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strjoin.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.co      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 01:49:55 by xalara            #+#    #+#             */
/*   Updated: 2026/08/10 12:05:59 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>

int	ft_strlen(char *str)
{
	int	i;

	i = 0;
	while (str[i])
		i++;
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

int	ft_intstrjoin(int size, char **strs, char *sep)
{
	int	i;
	int	c;
	int	l_sep;

	l_sep = ft_strlen(sep);
	c = 0;
	i = 0;
	while (i < size)
	{
		c += ft_strlen(strs[i]) + l_sep;
		i++;
	}
	c -= l_sep;
	return (c);
}

char	*ft_strjoin(int size, char **strs, char *sep)
{
	char	*r;
	int		i;
	int		j;
	int		c;

	c = 0;
	if (size > 0)
		c = ft_intstrjoin(size, strs, sep);
	r = (char *)malloc(sizeof(char) * (c + 1));
	if (!r)
		return (NULL);
	j = 0;
	i = 0;
	while (i < size - 1)
	{
		j += ft_strcpy(&r[j], strs[i]);
		j += ft_strcpy(&r[j], sep);
		i++;
	}
	if (size > 0)
		j += ft_strcpy(&r[j], strs[i]);
	r[j] = '\0';
	return (r);
}
