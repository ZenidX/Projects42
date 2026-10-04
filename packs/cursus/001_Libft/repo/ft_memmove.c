/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memmove.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/29 04:19:42 by xalara            #+#    #+#             */
/*   Updated: 2026/09/30 12:03:43 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memmove(void *dest, const void *src, size_t n)
{
	unsigned char		*p;
	const unsigned char	*q;
	size_t				i;

	if (dest == NULL && src == NULL)
		return (NULL);
	p = dest;
	q = src;
	if (dest > src)
	{
		i = n;
		while (i > 0)
		{
			i--;
			p[i] = q[i];
		}
	}
	else
		return (ft_memcpy(dest, src, n));
	return (dest);
}
