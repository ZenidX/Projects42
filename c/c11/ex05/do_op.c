/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   do_op.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 12:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/07/23 12:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include "do_op.h"

static int	ft_strcmp(char *a, char *b)
{
	int	i;

	i = 0;
	while (a[i] && a[i] == b[i])
		i++;
	return (a[i] - b[i]);
}

static int	is_zero_err(char *op, int b)
{
	if (b != 0)
		return (0);
	if (!ft_strcmp(op, "/"))
	{
		write(1, "Stop : division by zero\n", 24);
		return (1);
	}
	if (!ft_strcmp(op, "%"))
	{
		write(1, "Stop : modulo by zero\n", 22);
		return (1);
	}
	return (0);
}

static int	run(char *op, int a, int b)
{
	t_op	ops[5];
	int		i;

	ops[0].sym = "+";
	ops[0].f = &ft_add;
	ops[1].sym = "-";
	ops[1].f = &ft_sub;
	ops[2].sym = "*";
	ops[2].f = &ft_mul;
	ops[3].sym = "/";
	ops[3].f = &ft_div;
	ops[4].sym = "%";
	ops[4].f = &ft_mod;
	i = 0;
	while (i < 5)
	{
		if (!ft_strcmp(ops[i].sym, op))
			return (ops[i].f(a, b));
		i++;
	}
	return (0);
}

int	main(int argc, char **argv)
{
	int	a;
	int	b;

	if (argc != 4)
		return (0);
	a = ft_atoi(argv[1]);
	b = ft_atoi(argv[3]);
	if (is_zero_err(argv[2], b))
		return (0);
	ft_putnbr(run(argv[2], a, b));
	write(1, "\n", 1);
	return (0);
}
