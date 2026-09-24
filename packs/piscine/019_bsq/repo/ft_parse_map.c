/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_parse_map.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/11 00:24:51 by xalara            #+#    #+#             */
/*   Updated: 2026/08/11 06:53:22 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "bsq.h"

t_map	*ft_parse_error(t_map *map)
{
	ft_free_map(map);
	return (NULL);
}

/*
** En el cuerpo del mapa solo caben vacio y obstaculo. El caracter "lleno" lo
** pone el programa al pintar: si ya viene en el fichero, el mapa no es valido.
*/

int	ft_is_map(char c, t_map *map)
{
	if (c == map->obj[0])
		return (1);
	if (c == map->obj[1])
		return (2);
	return (0);
}

t_map	*ft_validate_map(t_map *map)
{
	int	i;
	int	j;

	map->width = ft_strlen(map->map[1]);
	if (map->width == 0)
		return (ft_parse_error(map));
	i = 1;
	while (i <= map->height)
	{
		if (ft_strlen(map->map[i]) != map->width)
			return (ft_parse_error(map));
		j = 0;
		while (j < map->width)
		{
			if (!ft_is_map(map->map[i][j], map))
				return (ft_parse_error(map));
			j++;
		}
		i++;
	}
	return (map);
}

/*
** El conteo de lineas no es opcional: ft_validate_map recorre hasta
** map[height], asi que si la cabecera promete mas lineas de las que hay se
** saldria por el NULL del split. Y de paso cubre el caso contrario, el de un
** fichero con lineas de sobra.
*/

t_map	*ft_parse_map(char *map_str)
{
	t_map	*map;

	map = (t_map *)malloc(sizeof(t_map));
	if (!map)
		return (NULL);
	map->sq[0] = 0;
	map->sq[1] = 0;
	map->sq[2] = 0;
	map->map = ft_split(map_str, "\n");
	if (!map->map || ft_arrlen(map->map) < 2)
		return (ft_parse_error(map));
	if (!ft_parse_header(map))
		return (ft_parse_error(map));
	if (ft_arrlen(map->map) != map->height + 1)
		return (ft_parse_error(map));
	return (ft_validate_map(map));
}
