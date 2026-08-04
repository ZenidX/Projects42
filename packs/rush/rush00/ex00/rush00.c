/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   rush00.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cristim2 <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/25 12:25:47 by cristim2          #+#    #+#             */
/*   Updated: 2026/07/26 15:02:47 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

void	ft_putchar(char c);

void	print_character(int i, int j, int x, int y)
{
	if ((i == 1 && j == 1)
		|| (i == y && j == x))
		ft_putchar('A');
	else if ((i == 1 && j == x)
		|| (i == y && j == 1))
		ft_putchar('C');
	else if (i == 1 || i == y
		|| j == 1 || j == x)
		ft_putchar('B');
	else
		ft_putchar(' ');
}

void	rush(int x, int y)
{
	int	i;
	int	j;

	if (x <= 0 || y <= 0)
		return ;
	i = 1;
	while (i <= y)
	{
		j = 1;
		while (j <= x)
		{
			print_character(i, j, x, y);
			j++;
		}
		ft_putchar('\n');
		i++;
	}
}
