/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_parse_header.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/11 07:36:00 by xalara            #+#    #+#             */
/*   Updated: 2026/08/11 07:36:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include "bsq.h"

int	ft_only_digits(char *s, int n)
{
	int	i;

	if (n < 1)
		return (0);
	i = 0;
	while (i < n)
	{
		if (s[i] < '0' || s[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

/* Los tres caracteres tienen que ser imprimibles y distintos entre si. */
int	ft_check_obj(t_map *map)
{
	int	i;

	i = 0;
	while (i < 3)
	{
		if (map->obj[i] < 32 || map->obj[i] > 126)
			return (0);
		i++;
	}
	if (map->obj[0] == map->obj[1] || map->obj[0] == map->obj[2]
		|| map->obj[1] == map->obj[2])
		return (0);
	return (1);
}

/*
** Cabecera: los tres ultimos caracteres son vacio, obstaculo y lleno, y todo
** lo de delante es el numero de lineas. Hacen falta al menos cuatro (un digito
** mas los tres caracteres) para que la cuenta salga.
*/

int	ft_parse_header(t_map *map)
{
	char	*h;
	int		len;

	h = map->map[0];
	len = ft_strlen(h);
	if (len < 4 || !ft_only_digits(h, len - 3))
		return (0);
	map->obj[0] = h[len - 3];
	map->obj[1] = h[len - 2];
	map->obj[2] = h[len - 1];
	if (!ft_check_obj(map))
		return (0);
	map->height = ft_is_pos(ft_atoi(h));
	return (map->height != 0);
}
