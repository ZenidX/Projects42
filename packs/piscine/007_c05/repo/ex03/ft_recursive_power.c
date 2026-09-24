/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_recursive_power.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/28 18:27:34 by xalara            #+#    #+#             */
/*   Updated: 2026/07/28 18:36:59 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

int	ft_recursive_power(int nb, int power)
{
	if (power > 1)
		return (nb * ft_recursive_power(nb, power - 1));
	else if (power == 1)
		return (nb);
	else if (power == 0)
		return (1);
	else
		return (0);
}
