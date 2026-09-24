/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_check_col.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 07:42:07 by xalara            #+#    #+#             */
/*   Updated: 2026/08/02 18:36:29 by ltejada          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
int	ft_check_col_top(int inp[4][9], int size, int pos[9][9], int p)
{
	int	i;
	int	max;
	int	count;

	i = 0;
	max = 0;
	count = 0;
	while (i < size)
	{
		if (max < pos[i][p])
		{
			max = pos[i][p];
			count++;
		}
		i++;
	}
	if (count == inp[0][p])
		return (1);
	else
		return (0);
}

int	ft_check_col_bottom(int inp[4][9], int size, int pos[9][9], int p)
{
	int	i;
	int	max;
	int	count;

	i = 0;
	max = 0;
	count = 0;
	while (i < size)
	{
		if (max < pos[size - i - 1][p])
		{
			max = pos[size - i - 1][p];
			count++;
		}
		i++;
	}
	if (count == inp[1][p])
		return (1);
	else
		return (0);
}

int	ft_check_col(int inp[4][9], int size, int pos[9][9])
{
	int	i;

	i = 0;
	while (i < size)
	{
		if (ft_check_col_top(inp, size, pos, i) == 0
			|| ft_check_col_bottom(inp, size, pos, i) == 0)
			return (0);
		i++;
	}
	return (1);
}
