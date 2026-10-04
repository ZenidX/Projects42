/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_calloc.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/02 01:34:19 by xalara            #+#    #+#             */
/*   Updated: 2026/10/04 03:21:41 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_calloc(size_t nmemb, size_t size)
{
	void	*p;
	long	s;

	if (nmemb == 0 || size == 0)
		return (malloc(0));
	s = nmemb * size;
	if (nmemb != 0 && s / nmemb != size)
		return (NULL);
	p = (void *)malloc(sizeof(char) * s);
	ft_bzero(p, nmemb * size);
	return (p);
}
