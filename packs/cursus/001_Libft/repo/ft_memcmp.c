/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memcmp.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/02 00:17:58 by xalara            #+#    #+#             */
/*   Updated: 2026/10/04 15:13:13 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_memcmp(const void *s1, const void *s2, size_t n)
{
	unsigned char	*p;
	unsigned char	*q;
	size_t			i;

	if (n == 0)
		return (0);
	p = (unsigned char *) s1;
	q = (unsigned char *) s2;
	i = 0;
	while (i < n && p[i] == q[i])
		i++;
	if (i != n)
		return ((unsigned char) p[i] - (unsigned char) q[i]);
	return (0);
}
