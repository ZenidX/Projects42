/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_fibonacci.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 18:38:23 by xalara            #+#    #+#             */
/*   Updated: 2026/07/28 20:24:25 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

int	ft_fibonacci(int index)
{
	int	a;
	int	b;
	int	i;

	a = 0;
	b = 1;
	i = 2;
	while (i <= index)
	{
		if (i % 2 == 0)
			a = a + b;
		else
			b = a + b;
		i++;
	}
	if (index % 2 == 0)
		return (a);
	else
		return (b);
}
/*
void	ft_putint(int n)
{
	char	d;

	if (n < 0)
	{
		d = '-';
		write(1, &d, 1);
		n = -n;
	}
	if (n / 10 > 0)
	{
		ft_putint(n / 10);
	}
	d = n % 10 + '0';
	write(1, &d, 1);
}

int	main(void)
{
	int	i;

	i = 0;
	while (i < 10)
	{
		ft_putint(ft_fibonacci(i));
		write(1, " ", 1);
		i++;
	}
}
*/
