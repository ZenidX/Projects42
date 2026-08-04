/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_range.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.co      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 01:35:31 by xalara            #+#    #+#             */
/*   Updated: 2026/08/04 01:48:37 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

int	*ft_range(int min, int max)
{
	int	*r;
	int	i;

	if (max <= min)
		return (NULL);
	r = malloc(sizeof(int) * (max - min));
	i = 0;
	while (i < max - min)
	{
		r[i] = min + i;
		i++;
	}
	return (r);
}
