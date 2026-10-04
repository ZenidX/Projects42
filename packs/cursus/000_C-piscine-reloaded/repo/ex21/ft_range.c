/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_range.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 17:21:35 by xalara            #+#    #+#             */
/*   Updated: 2026/09/24 18:45:43 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>

int	*ft_range(int min, int max)
{
	int	*r;
	int	i;

	if (max - min < 1)
		return (NULL);
	r = (int *) malloc(sizeof(int) * (max - min));
	i = 0;
	while (i < max - min)
	{
		r[i] = min + i;
		i++;
	}
	return (r);
}
