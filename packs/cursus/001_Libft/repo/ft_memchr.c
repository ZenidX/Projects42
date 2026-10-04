/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_memchr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/02 00:09:29 by xalara            #+#    #+#             */
/*   Updated: 2026/10/02 00:45:59 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memchr(const void *s, int c, size_t n)
{
	char	*p;
	size_t	i;

	p = (char *) s;
	i = 0;
	while (p[i] != (char)c && i < n)
		i++;
	if (p[i] != (char)c || i == n)
		return (NULL);
	else
		return (&p[i]);
}
