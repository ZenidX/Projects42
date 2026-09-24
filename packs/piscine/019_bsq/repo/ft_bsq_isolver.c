/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_bsq_isolver.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/11 05:10:42 by xalara            #+#    #+#             */
/*   Updated: 2026/08/11 06:22:29 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include "bsq.h"

void	ft_validate_sq(t_map *map, int i[2], int *tmp)
{
	int	j[2];

	j[0] = 0;
	while (j[0] < *tmp)
	{
		j[1] = 0;
		while (j[1] < *tmp)
		{
			if (map->map[i[0] + j[0]][i[1] + j[1]] == map->obj[1])
				*tmp = -1;
			j[1]++;
		}
		j[0]++;
	}
	if (map->sq[2] < *tmp)
	{
		map->sq[0] = i[0];
		map->sq[1] = i[1];
		map->sq[2] = *tmp;
	}
}

void	ft_bsq_isolver(t_map *map)
{
	int	i[2];
	int	tmp;

	i[0] = 1;
	while (i[0] <= map->height)
	{
		i[1] = 0;
		while (i[1] < map->width)
		{
			if (map->map[i[0]][i[1]] == map->obj[0])
			{
				tmp = map->sq[2];
				while (tmp != -1
					&& tmp <= map->height - i[0]
					&& tmp < map->width - i[1])
				{
					tmp++;
					ft_validate_sq(map, i, &tmp);
				}
			}
			i[1]++;
		}
		i[0]++;
	}
}
