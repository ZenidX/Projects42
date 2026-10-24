/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa_base.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/18 14:30:00 by xalara            #+#    #+#             */
/*   Updated: 2026/10/18 14:30:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static char	*ft_char_init(char *s, int size)
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

char	*ft_itoa_base(unsigned long n, char *base, int b, int d)
{
	char	*r;
	int		dd;

	if (n / b != 0)
		r = ft_itoa_base(n / b, base, b, d + 1);
	else
	{
		r = (char *)malloc(sizeof(char) * (d + 2));
		r = ft_char_init(r, d + 1);
	}
	if (!r)
		return (NULL);
	dd = ft_strlen(r);
	r[dd - 1 - d] = base[n % b];
	return (r);
}
