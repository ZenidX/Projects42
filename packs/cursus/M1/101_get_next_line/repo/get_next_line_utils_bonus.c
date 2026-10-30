/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils_bonus.c                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/06 12:24:37 by xalara            #+#    #+#             */
/*   Updated: 2026/10/17 03:41:53 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line_bonus.h"

size_t	gnl_strlen(char *r)
{
	size_t	i;

	if (!r)
		return (0);
	i = 0;
	while (r && r[i])
		i++;
	return (i);
}

char	*gnl_strchr(char *r, char c)
{
	int	i;
	int	l;

	l = gnl_strlen(r);
	i = 0;
	while (i < l && r && r[i] != c)
		i++;
	if (i == l || !r)
		return (NULL);
	else
		return (&r[i]);
}

char	*gnl_strdup(char *s, size_t l)
{
	char	*r;
	size_t	i;

	r = malloc(sizeof(char) * (l + 1));
	if (!r)
		return (NULL);
	i = 0;
	while (i < l && s[i])
	{
		r[i] = s[i];
		i++;
	}
	while (i < l + 1)
	{
		r[i] = '\0';
		i++;
	}
	return (r);
}

void	gnl_strncpy(char *dst, char *q, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n && q[i])
	{
		dst[i] = q[i];
		i++;
	}
	while (i < n && dst[i])
	{
		dst[i] = '\0';
		i++;
	}
}

char	*gnl_strncat(char *dst, char *src, size_t n)
{
	char	*r;
	size_t	l_d;
	size_t	l_s;
	size_t	i;

	l_d = gnl_strlen(dst);
	l_s = gnl_strlen(src);
	r = gnl_strdup(dst, n);
	free(dst);
	if (!r)
		return (NULL);
	i = 0;
	while (i < l_s && i + l_d < n && src[i])
	{
		r[l_d + i] = src[i];
		i++;
	}
	r[l_d + i] = '\0';
	return (r);
}
