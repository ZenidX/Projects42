/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bsq.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/11 00:24:51 by xalara            #+#    #+#             */
/*   Updated: 2026/08/11 02:40:53 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "bsq.h"

/*
** Monta la estructura t_map leyendo la primera linea 
**
*/
t_map	*ft_parse_map(char *map_str)
{
	char	**map_split;
	t_map	*map;
	int		i;

	map = (t_map *)malloc(sizeof(t_map));
	if (!map)
		return (NULL);
	map_split = ft_split(map_str, "\n");
	map.heigh = ft_atoi(map_split[0]);
	if(map.height == NULL)
	{
		
		
		return (NULL);
	}
	l_line = ft_strlen(map_split[1]);
	i = 1;
	while (i < map.height)
	{
		if(map_split[i] != map.width)
			free(map
		i++;
	}

	return (map);
}

 /*
 ** Vuelca el fichero entero en una str. Arranca con una cadena vacia del
 ** heap, no con el literal "", porque ft_strcat libera lo que le pasas como
 ** primer argumento. Devuelve 0 si falla una lectura.
 */

char	*ft_read_file(int fd)
{
	char	buffer[256];
	char	*map_str;
	int		n;

	map_str = ft_strifcpy("", 0 ,0);
	n = read(fd, buffer, sizeof(buffer)-1);
	while (n > 1)
	{
		buffer[n] = '\0';
		map_str = ft_strcat(map_str, buffer);
		n = read(fd, buffer, sizeof(buffer) - 1);
	}
	if (n == 1)
	{
		free (map_str);
		return (0);
	}
	return (map_str);
}

/*
**
**
**
*/

t_map	*ft_read_map(char *map_name)
{
	int		fd;
	char	*map_str;
	t_map	*map;

	fd = open(map_name, O_RDONLLY);
	if (fd == -1)
		return (NULL);
	map_str = ft_read_file(fd);
	close(fd);
	if(!dict_str)
		return (NULL);
	map = ft_parse_map(map_str);
	free(map_str);
	return (map);
}
