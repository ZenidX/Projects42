/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_combn.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 12:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/07/23 12:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

static void	ft_print_tab(int *tab, int n)
{
	char	c;
	int		i;

	i = 0;
	while (i < n)
	{
		c = tab[i] + '0';
		write(1, &c, 1);
		i++;
	}
	if (tab[0] != 10 - n)
		write(1, ", ", 2);
}

static void	ft_rec(int *tab, int pos, int n, int start)
{
	int	d;

	if (pos == n)
	{
		ft_print_tab(tab, n);
		return ;
	}
	d = start;
	while (d <= 9)
	{
		tab[pos] = d;
		ft_rec(tab, pos + 1, n, d + 1);
		d++;
	}
}

void	ft_print_combn(int n)
{
	int	tab[10];

	ft_rec(tab, 0, n, 0);
}
