/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_convert_base2.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/06 13:16:18 by xalara            #+#    #+#             */
/*   Updated: 2026/08/06 14:55:24 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>

int	ft_strlen(char *s)
{
	int	i;

	i = 0;
	while (s[i])
	{
		i++;
	}
	return (i);
}

char	*ft_char_init(char *s, int size)
{
	int	i;

	if (!s)
		return (NULL);
	i = 0;
	while (i < size)
	{
		s[i] = '0';
		i++;
	}
	s[i] = '\0';
	return (s);
}
