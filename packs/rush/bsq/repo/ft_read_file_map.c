/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_read_file_map.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/11 00:24:51 by xalara            #+#    #+#             */
/*   Updated: 2026/08/11 04:20:53 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "bsq.h"

char	*ft_read_file(int fd)
{
	char	buffer[256];
	char	*map_str;
	int		n;

	map_str = ft_strifcpy("", 0, 0);
	n = read(fd, buffer, sizeof(buffer) - 1);
	while (n > 0)
	{
		buffer[n] = '\0';
		map_str = ft_strfreecat(map_str, buffer);
		if (!map_str)
			return (0);
		n = read(fd, buffer, sizeof(buffer) - 1);
	}
	if (n < 0)
	{
		free(map_str);
		return (0);
	}
	return (map_str);
}

/*
** Vuelca el descriptor y lo parsea. Separado de ft_read_map porque sin
** argumentos el mapa entra por la entrada estandar, y ahi no hay que abrir
** ni cerrar nada: el descriptor ya viene dado.
*/

t_map	*ft_map_from_fd(int fd)
{
	char	*map_str;
	t_map	*map;

	map_str = ft_read_file(fd);
	if (!map_str)
		return (NULL);
	map = ft_parse_map(map_str);
	free(map_str);
	return (map);
}

t_map	*ft_read_map(char *map_name)
{
	int		fd;
	t_map	*map;

	fd = open(map_name, O_RDONLY);
	if (fd == -1)
		return (NULL);
	map = ft_map_from_fd(fd);
	close(fd);
	return (map);
}
