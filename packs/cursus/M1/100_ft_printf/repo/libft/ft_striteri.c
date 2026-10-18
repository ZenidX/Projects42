/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_striteri.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/03 23:53:12 by xalara            #+#    #+#             */
/*   Updated: 2026/10/04 04:16:07 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_striteri(char *s, void (*f)(unsigned int, char *))
{
	size_t	i;
	size_t	l;

	if (s && f)
	{
		l = ft_strlen(s);
		i = 0;
		while (i < l)
		{
			f((unsigned int)i, &s[i]);
			i++;
		}
	}
}
