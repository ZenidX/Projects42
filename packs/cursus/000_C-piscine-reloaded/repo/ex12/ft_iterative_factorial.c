/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_iterative_factorial.c                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 14:43:35 by xalara            #+#    #+#             */
/*   Updated: 2026/09/24 14:56:09 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

int	ft_iterative_factorial(int nb)
{
	int	i;
	int	j;

	if (nb < 0)
		return (0);
	i = 1;
	j = 1;
	while (nb >= j)
	{
		i = i * j;
		j++;
	}
	return (i);
}
