/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlcat.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 06:21:56 by xalara            #+#    #+#             */
/*   Updated: 2026/09/29 07:32:47 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

size_t	ft_strlcat(char *dst, const char *src, size_t siz)
{
	size_t	n_dst;
	size_t	i;

	n_dst = 0;
	while (n_dst < siz && dst[n_dst])
		n_dst++;
	if (n_dst == siz)
		return (siz + ft_strlen(src));
	n_dst = ft_strlen(dst);
	i = 0;
	while (i + n_dst < siz - 1 && src[i])
	{
		dst[n_dst + i] = src[i];
		i++;
	}
	dst[n_dst + i] = '\0';
	return (n_dst + ft_strlen(src));
}
