/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putmatrix.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 06:09:26 by xalara            #+#    #+#             */
/*   Updated: 2026/08/02 12:03:46 by ltejada          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include <unistd.h>

void	ft_putmatrix(int matrix[9][9], int size)
{
	char	d;
	int		i;
	int		j;

	i = 0;
	while (i < size)
	{
		j = 0;
		while (j < size)
		{
			d = matrix[i][j] + '0';
			write(1, &d, 1);
			j++;
			if (j < size)
				write(1, " ", 1);
			else
				write(1, "\n", 1);
		}
		i++;
	}
}
