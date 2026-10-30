/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memcpy.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 04:01:28 by xalara            #+#    #+#             */
/*   Updated: 2026/09/29 04:18:22 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memcpy(void *dest, const void *src, size_t n)
{
	unsigned char		*p;
	const unsigned char	*q;
	size_t				i;

	if (dest == NULL && src == NULL)
		return (dest);
	p = dest;
	q = src;
	i = 0;
	while (i < n)
	{
		p[i] = q[i];
		i++;
	}
	return (dest);
}
