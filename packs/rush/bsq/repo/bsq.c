/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bsq.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/11 02:41:17 by xalara            #+#    #+#             */
/*   Updated: 2026/08/11 07:10:23 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include "bsq.h"

/*
** Un mapa de principio a fin. Con map a NULL el fichero no se pudo leer o no
** era valido, y lo unico que toca es el mensaje: ni resolver ni pintar.
*/

void	ft_run_map(t_map *map)
{
	if (!map)
	{
		ft_putstr("map error\n");
		return ;
	}
	ft_bsq_isolver(map);
	ft_print_map(map);
	ft_free_map(map);
}

/*
** Sin argumentos se lee un unico mapa de la entrada estandar. Con ellos, uno
** por fichero, con una linea en blanco ENTRE dos salidas: por eso el salto va
** antes de cada mapa menos del primero, y no despues de cada uno.
*/

int	main(int argc, char **argv)
{
	int	i;

	if (argc < 2)
	{
		ft_run_map(ft_map_from_fd(0));
		return (0);
	}
	i = 1;
	while (i < argc)
	{
		if (i > 1)
			ft_putstr("\n");
		ft_run_map(ft_read_map(argv[i]));
		i++;
	}
	return (0);
}
