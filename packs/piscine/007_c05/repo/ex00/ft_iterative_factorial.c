/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_iterative_factorial.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 18:03:29 by xalara            #+#    #+#             */
/*   Updated: 2026/08/04 21:29:52 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

int	ft_iterative_factorial(int nbr)
{
	int	p;

	p = 1;
	while (nbr > 0)
	{
		p *= nbr;
		nbr--;
	}
	if (nbr < 0)
		return (0);
	return (p);
}
