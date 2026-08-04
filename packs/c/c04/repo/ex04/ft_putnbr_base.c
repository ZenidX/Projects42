/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putnbr_base.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 17:02:16 by xalara            #+#    #+#             */
/*   Updated: 2026/07/31 00:23:30 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

int	ft_base_ok(char *b)
{
	int	i;

	if (!b[0] || !b[1])
		return (0);
	i = 0;
	while (b[i])
	{
		if (b[i] == '+' || b[i] == '-' || b[i] == ' ')
			return (0);
		j = i + 1;
		while (b[j])
		{
			if (b[i] == b[j])
				return (0);
			j++;
		}
		i++;
	}
	return (1);
}

void	ft_putnbr_base(int nbr, char *base)
{
	int	i;
	int	d;
	int	b;

	if (ft_base_ok(base) == 1)
	{
		b = 0;
		while (base[b])
			b++;
		if (nbr < 0)
		{
			d = '-';
			write(1, &d, 1);
			nbr = -nbr;
		}
		if (nbr / b > 0)
		{
			ft_putnbr_base(nbr, base);
		}
		d = nbr % b;
		write (1, &base[d], 1);
	}
}
