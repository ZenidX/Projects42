/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line_utils.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/06 12:24:37 by xalara            #+#    #+#             */
/*   Updated: 2026/10/10 17:33:32 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

size_t	gnl_strlen(char *r)
{
	size_t	i;

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

void	gnl_strifcpy(char *dst, char *src, size_t ini, size_t fin)
{
	size_t	i;

	i = 0;
	while (i + ini < fin && src[i])
	{
		dst[ini + i] = src[i];
		i++;
	}
	dst[i] = '\0';
}

char	*gnl_strcat(char *dst, char *src)
{
	char 	*r;
	size_t	l_d;
	size_t	l_s;

	l_d = ft_strlen(dst);
	l_s = ft_strlen(src);
	r = (char *)malloc(sizeof(char) * (l_d + l_s + 1));
	if (!r)
		return (NULL);
	gnl_strifcpy(r, dst, 0, l_d);
	free(dst);
	gnl_strifcpy(r, src, l_d, l_d + l_s + 1);
	return (r);
}
