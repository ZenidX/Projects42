/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_check_row.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 07:42:07 by xalara            #+#    #+#             */
/*   Updated: 2026/08/02 15:55:35 by ltejada          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
int	ft_check_row_left(int inp[4][9], int size, int pos[9][9], int p)
{
	int	i;
	int	max;
	int	count;

	i = 0;
	max = 0;
	count = 0;
	while (i < size)
	{
		if (max < pos[p][i])
		{
			max = pos[p][i];
			count++;
		}
		i++;
	}
	if (count == inp[2][p])
		return (1);
	else
		return (0);
}

int	ft_check_row_right(int inp[4][9], int size, int pos[9][9], int p)
{
	int	i;
	int	max;
	int	count;

	i = 0;
	max = 0;
	count = 0;
	while (i < size)
	{
		if (max < pos[p][size - i - 1])
		{
			max = pos[p][size - i - 1];
			count++;
		}
		i++;
	}
	if (count == inp[3][p])
		return (1);
	else
		return (0);
}
