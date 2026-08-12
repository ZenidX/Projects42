/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_map.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/11 07:14:56 by xalara            #+#    #+#             */
/*   Updated: 2026/08/11 07:25:29 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include "bsq.h"

/*
** Sustituye por el caracter "lleno" las celdas del mayor cuadrado, sobre el
** propio mapa. sq[0] es la fila y sq[1] la columna de la esquina superior
** izquierda, y sq[2] el lado. Si no se encontro ningun cuadrado el lado es 0 y
** los dos bucles no entran, asi que el mapa sale tal cual.
*/

void	ft_fill_sq(t_map *map)
{
	int	i;
	int	j;

	i = 0;
	while (i < map->sq[2])
	{
		j = 0;
		while (j < map->sq[2])
		{
			map->map[map->sq[0] + i][map->sq[1] + j] = map->obj[2];
			j++;
		}
		i++;
	}
}

/*
** Pinta el cuadrado y saca las lineas enteras. Una linea es un solo write en
** vez de uno por celda: en el mapa de 300x300 son 600 llamadas al sistema en
** lugar de 90000.
*/

void	ft_print_map(t_map *map)
{
	int	i;

	ft_fill_sq(map);
	i = 1;
	while (i <= map->height)
	{
		ft_putstr(map->map[i]);
		ft_putstr("\n");
		i++;
	}
}
