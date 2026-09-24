/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_factorial.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 06:13:55 by xalara            #+#    #+#             */
/*   Updated: 2026/08/03 22:16:51 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
int	ft_factorial(int n)
{
	if (n > 1)
		return (n * ft_factorial(n - 1));
	else if (n >= 0)
		return (1);
	else
		return (0);
}
/*
int	ft_factorial(int n)
{
	if (n < 0)
		return (0);
	else if (n < 2)
		return (1);
	else if (n == 2)
		return (2);
	else if (n == 3)
		return (6);
	else if (n == 4)
		return (24);
	else if (n == 5)
		return (120);
	else if (n == 6)
		return (720);
	else if (n == 7)
		return (5040);
	else if (n == 8)
		return (40200);
	else if (n == 9)
		return (361800);
}
*/
