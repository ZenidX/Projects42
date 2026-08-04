/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ten_queens_puzzle.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 21:18:37 by xalara            #+#    #+#             */
/*   Updated: 2026/07/29 14:14:28 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

void	ft_intcpy(int *dest, int *src)
{
	int	i;

	i = 0;
	while (i < 10)
	{
		dest[i] = src[i];
		i++;
	}
}

int	ft_q_available(int *pos, int n, int p)
{
	int	i;

	i = -n;
	while (!(i + p > 0))
		i++;
	while (i + p < n)
	{
		if (i != 0
			&& (pos[p + i] + i == pos[p]
				|| pos[p + i] - i == pos[p]
				|| pos[p + i] == pos[p]))
			return (0);
		i++;
	}
	return (1);
}

void	ft_putdig(int *pos, int i, int n)
{
	char	d;

	while (i < n)
	{
		d = pos[i] + '0';
		write(1, &d, 1);
		i++;
	}
	d = 10;
	write(1, &d, 1);
}

int	ft_q_isolver(int *poz, int n, int p)
{
	int		pos[10];
	int		i;
	int		c;

	ft_intcpy(pos, poz);
	c = 0;
	i = 0;
	while (i < n && p < n)
	{
		pos[p] = i;
		if (ft_q_available(pos, n, p) == 1)
			c += ft_q_isolver(pos, n, p + 1);
		i++;
	}
	if (p == n)
	{
		ft_putdig(pos, i, n);
		c = 1;
	}
	return (c);
}

int	ft_ten_queens_puzzle(void)
{
	int	pos[10];
	int	i;

	i = 0;
	while (i < 10)
	{
		pos[i] = -100;
		i++;
	}
	return (ft_q_isolver(pos, i, 0));
}
/*
int	main(void)
{
	ft_ten_queens_puzzle();
	return (0);
}
*/
