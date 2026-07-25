/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strjoin.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 12:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/07/23 12:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>

static int	ft_strlen(char *s)
{
	int	i;

	i = 0;
	while (s[i])
		i++;
	return (i);
}

static int	total_len(int size, char **strs, char *sep)
{
	int	len;
	int	i;

	if (size == 0)
		return (0);
	len = ft_strlen(sep) * (size - 1);
	i = 0;
	while (i < size)
	{
		len += ft_strlen(strs[i]);
		i++;
	}
	return (len);
}

static char	*ft_append(char *dst, char *src)
{
	int	i;

	i = 0;
	while (src[i])
	{
		*dst = src[i];
		dst++;
		i++;
	}
	return (dst);
}

char	*ft_strjoin(int size, char **strs, char *sep)
{
	char	*res;
	char	*p;
	int		i;

	res = (char *)malloc(sizeof(char) * (total_len(size, strs, sep) + 1));
	if (!res)
		return (NULL);
	p = res;
	i = 0;
	while (i < size)
	{
		p = ft_append(p, strs[i]);
		if (i < size - 1)
			p = ft_append(p, sep);
		i++;
	}
	*p = '\0';
	return (res);
}
