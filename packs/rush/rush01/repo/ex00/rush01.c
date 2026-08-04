/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   rush01.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltejada <ltejada@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/01 10:51:21 by ltejada           #+#    #+#             */
/*   Updated: 2026/08/02 19:10:59 by ltejada          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

int		ft_is_valid_arg(char *str);

int		ft_parse_arg(int inp[4][9], char *str, int size);

void	ft_putmatrix(int matrix[9][9], int size);

int		ft_factorial(int n);

int		ft_check_col(int inp[4][9], int size, int pos[9][9]);

int		ft_check_row_right(int inp[4][9], int size, int pos[9][9], int p);

int		ft_check_row_left(int inp[4][9], int size, int pos[9][9], int p);

int		ft_diff_col(int pos[9][9], int size);

void	ft_ordcom(int ava[9], int pos[9][9], int i, int p)
{
	int	t;
	int	j;
	int	i_ava;
	int	size;

	size = 0;
	while (ava[size] == 0)
		size++;
	j = 0;
	while (j < size)
	{
		t = i / ft_factorial(size - 1 - j);
		i_ava = 0;
		while (i_ava < size && t >= 0)
		{
			if (ava[i_ava] == 0)
				t--;
			i_ava++;
		}
		ava[i_ava - 1] = 1;
		pos[p][j] = i_ava;
		i = i % ft_factorial(size - 1 - j);
		j++;
	}
}

void	ft_setcom(int pos[9][9], int size, int i_com, int p)
{
	int	ava[9];
	int	i;

	i = 0;
	while (i < 9)
	{
		if (i < size)
			ava[i] = 0;
		else
			ava[i] = 1;
		i++;
	}
	ft_ordcom(ava, pos, i_com, p);
}

int	ft_skyline_isolver(int inp[4][9], int size, int pos[9][9], int p)
{
	int	i_com;
	int	r;

	r = 0;
	i_com = 0;
	while (i_com < ft_factorial(size) && p < size)
	{
		ft_setcom(pos, size, i_com, p);
		if (ft_check_row_left(inp, size, pos, p)
			&& ft_check_row_right(inp, size, pos, p))
		{
			r = ft_skyline_isolver(inp, size, pos, p + 1);
			if (r == 1)
				return (r);
		}
		i_com++;
	}
	if (p == size)
		r = ft_check_col(inp, size, pos) && ft_diff_col(pos, size);
	else
		r = 0;
	return (r);
}

int	main(int argc, char **argv)
{
	int	inp[4][9];
	int	size;
	int	sol[9][9];

	if (argc != 2)
	{
		write(1, "Error\n", 6);
		return (1);
	}
	size = ft_is_valid_arg(argv[1]);
	if (size < 3 || size > 10)
	{
		write(1, "Error\n", 6);
		return (1);
	}
	ft_parse_arg(inp, argv[1], size);
	if (ft_skyline_isolver(inp, size, sol, 0))
		ft_putmatrix(sol, size);
	else
	{
		write(1, "Error\n", 6);
		return (1);
	}
	return (0);
}
