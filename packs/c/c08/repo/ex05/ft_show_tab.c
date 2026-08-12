/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_show_tab.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/10 11:35:47 by xalara            #+#    #+#             */
/*   Updated: 2026/08/12 12:28:23 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include "ft_stock_str.h"

int	ft_strlen1(char *str)
{
	int	i;

	i = 0;
	while (str[i])
		i++;
	return (i);
}

void	ft_putstr(char *str)
{
	write(1, str, ft_strlen1(str));
}

void	ft_putnbr(int i)
{
	char	d;

	if (i < 0)
	{
		if (i < -10)
		{
			d = (-(i + 10)) % 10 + '0';
		}
		else
		{
			if (-i / 10 > 0)
				ft_putnbr(-i / 10);
			d = -i % 10 + '0';
		}
		write(1, &d, 1);
	}
	else
	{
		if (i / 10 > 0)
			ft_putnbr(i / 10);
		d = i % 10 + '0';
		write(1, &d, 1);
	}
}

void	ft_show_tab(struct s_stock_str *par)
{
	int	i;

	i = 0;
	while (par[i].size)
	{
		ft_putstr(par[i].str);
		write(1, "\n", 1);
		ft_putnbr(par[i].size);
		write(1, "\n", 1);
		ft_putstr(par[i].copy);
		write(1, "\n", 1);
		i++;
	}
}
