/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_find_next_prime.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 20:33:01 by xalara            #+#    #+#             */
/*   Updated: 2026/07/28 21:17:31 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

bool	ft_is_prime(int n)
{
	int	i;

	i = 2;
	while (i < n)
	{
		if (n % i == 0)
			return (false);
		i++;
	}
	return (true);
}

int	ft_find_next_prime(int nb)
{
	while (ft_is_prime(nb))
		nb++;
	return (nb);
}
