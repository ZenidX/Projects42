/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ten_queens_puzzle.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 12:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/07/23 12:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

static int	is_valid(int *board, int col, int row)
{
	int	i;

	i = 0;
	while (i < col)
	{
		if (board[i] == row)
			return (0);
		if (col - i == row - board[i] || col - i == board[i] - row)
			return (0);
		i++;
	}
	return (1);
}

static void	print_board(int *board)
{
	char	line[11];
	int		i;

	i = 0;
	while (i < 10)
	{
		line[i] = board[i] + '0';
		i++;
	}
	line[10] = '\n';
	write(1, line, 11);
}

static int	solve(int *board, int col)
{
	int	row;
	int	count;

	if (col == 10)
	{
		print_board(board);
		return (1);
	}
	row = 0;
	count = 0;
	while (row < 10)
	{
		if (is_valid(board, col, row))
		{
			board[col] = row;
			count += solve(board, col + 1);
		}
		row++;
	}
	return (count);
}

int	ft_ten_queens_puzzle(void)
{
	int	board[10];

	return (solve(board, 0));
}
