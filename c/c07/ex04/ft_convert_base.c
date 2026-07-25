/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_convert_base.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 12:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/07/23 12:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>

int		ft_is_valid_base(char *base);
long	ft_atoi_base(char *nbr, char *base);
int		ft_baselen(char *base);

static int	nbr_len(long n, int base_len)
{
	int	len;

	len = 1;
	while (n >= base_len)
	{
		n /= base_len;
		len++;
	}
	return (len);
}

static char	*fill_result(long n, char *base, int base_len)
{
	char	*res;
	int		len;
	int		neg;

	neg = (n < 0);
	if (neg)
		n = -n;
	len = nbr_len(n, base_len) + neg;
	res = (char *)malloc(sizeof(char) * (len + 1));
	if (!res)
		return (NULL);
	res[len] = '\0';
	while (len > neg)
	{
		len--;
		res[len] = base[n % base_len];
		n /= base_len;
	}
	if (neg)
		res[0] = '-';
	return (res);
}

char	*ft_convert_base(char *nbr, char *base_from, char *base_to)
{
	long	value;

	if (!ft_is_valid_base(base_from) || !ft_is_valid_base(base_to))
		return (NULL);
	value = ft_atoi_base(nbr, base_from);
	return (fill_result(value, base_to, ft_baselen(base_to)));
}
