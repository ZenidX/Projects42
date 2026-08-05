/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ten_queens_puzzle.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/05 11:48:54 by xalara            #+#    #+#             */
/*   Updated: 2026/08/05 16:14:01 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

int	ft_q_ava(int pos[10], int p)
{
	int	i;

	i = 0;
	while (i < 10)
	{
		if ((pos[i] == pos[p]
				|| pos[i] + p - i == pos[p]
				|| pos[i] - p + i == pos[p])
			&& i != p)
			return (0);
		i++;
	}
	return (1);
}

void	ft_putpos(int pos[10])
{
	char	d;
	int		i;

	i = 0;
	while (i < 10)
	{
		d = pos[i] + '0';
		write(1, &d, 1);
		i++;
	}
	write(1, "\n", 1);
}

void	ft_intcpy(int dest[10], int src[10])
{
	int	i;

	i = 0;
	while (i < 10)
	{
		dest[i] = src[i];
		i++;
	}
}

int	ft_q_isolver(int poz[10], int p)
{
	int	pos[10];
	int	i;
	int	r;

	ft_intcpy(pos, poz);
	r = 0;
	i = 0;
	while (i < 10 && p != 10)
	{
		pos[p] = i;
		if (ft_q_ava(pos, p))
			r += ft_q_isolver(pos, p + 1);
		i++;
	}
	if (p == 10)
	{
		ft_putpos(pos);
		return (1);
	}
	return (r);
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
	return (ft_q_isolver(pos, 0));
}
