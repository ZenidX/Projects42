/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_itoa.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/02 03:48:45 by xalara            #+#    #+#             */
/*   Updated: 2026/10/04 15:29:48 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static char	*ft_char_init(char *r, int d)
{
	int	i;

	i = 0;
	while (i < d)
	{
		r[i] = '0';
		i++;
	}
	r[i] = '\0';
	return (r);
}

static char	*ft_itoa_real(int n, int d)
{
	char	*r;
	int		dd;
	int		neg;

	neg = (n < 0);
	if (n / 10 != 0)
		r = ft_itoa_real(n / 10, d + 1);
	else
	{
		r = (char *)malloc(sizeof(char) * (d + 2 + neg));
		if (!r)
			return (NULL);
		r = ft_char_init(r, d + 1 + neg);
	}
	if (!r)
		return (NULL);
	dd = ft_strlen(r);
	if (neg)
	{
		r[0] = '-';
		r[dd - 1 - d] = -(n % 10) + '0';
	}
	else
		r[dd - 1 - d] = n % 10 + '0';
	return (r);
}

char	*ft_itoa(int n)
{
	return (ft_itoa_real(n, 0));
}
