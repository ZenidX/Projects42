/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putnbr_base.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 17:02:16 by xalara            #+#    #+#             */
/*   Updated: 2026/08/04 19:36:40 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

void	ft_putnbr_base(int nbr, char *base);

int	ft_base_ok(char *b)
{
	int	i;
	int	j;

	if (!b[0])
		return (0);
	if (!b[1])
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
	return (i);
}

void	ft_putnbr_base_max(int nbr, char *base, int b, int *d)
{
	write(1, "-", 1);
	ft_putnbr_base(-(nbr + b) / b + 1, base);
	*d = -(nbr + b) % b;
}

void	ft_putnbr_base(int nbr, char *base)
{
	int	d;
	int	b;

	b = ft_base_ok(base);
	if (b)
	{
		if (nbr == -2147483648)
			ft_putnbr_base_max(nbr, base, b, &d);
		else
		{
			if (nbr < 0)
			{
				write(1, "-", 1);
				nbr = -nbr;
			}
			if (nbr / b > 0)
			{
				ft_putnbr_base(nbr / b, base);
			}
			d = nbr % b;
		}
		write (1, &base[d], 1);
	}
}
