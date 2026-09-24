/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_read_dict.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 03:47:57 by xalara            #+#    #+#             */
/*   Updated: 2026/08/09 20:30:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "rush02.h"

/*
** Recorta los espacios de los dos extremos. fin es la posicion siguiente al
** ultimo caracter, asi que el corte de atras mira str[fin - 1]. Si la cadena
** es toda espacios, ini y fin acaban valiendo lo mismo y ft_strifcpy devuelve
** una cadena vacia reservada, que se puede liberar igual que las demas.
*/
char	*ft_outspace(char *str)
{
	int	ini;
	int	fin;

	ini = 0;
	while (str[ini] && str[ini] == ' ')
		ini++;
	fin = ft_strlen(str);
	while (fin > ini && str[fin - 1] == ' ')
		fin--;
	return (ft_strifcpy(str, ini, fin));
}

/*
** Vuelca el fichero entero en una cadena. Arranca con una cadena vacia del
** heap, no con el literal "", porque ft_strcat libera lo que le pasas como
** primer argumento. Devuelve 0 si falla una lectura.
*/
char	*ft_read_file(int fd)
{
	char	buffer[255];
	char	*dict_str;
	int		n;

	dict_str = ft_strifcpy("", 0, 0);
	n = read(fd, buffer, sizeof(buffer) - 1);
	while (n > 0)
	{
		buffer[n] = '\0';
		dict_str = ft_strcat(dict_str, buffer);
		n = read(fd, buffer, sizeof(buffer) - 1);
	}
	if (n == -1)
	{
		free(dict_str);
		return (0);
	}
	return (dict_str);
}

/*
** Camino del Dict Error: cierra el diccionario en la entrada n para que
** ft_free_dict sepa donde parar, suelta todo lo del parseo y devuelve NULL.
*/
t_dict	*ft_parse_error(t_dict *dict, char **lines, char **split, int n)
{
	dict[n].nbr = 0;
	ft_free_split(split);
	ft_free_split(lines);
	ft_free_dict(dict);
	return (NULL);
}

t_dict	*ft_parse_dict(char *dict_str)
{
	char	**dict_lines;
	char	**dict_split;
	t_dict	*dict;
	int		i;

	dict_lines = ft_split(dict_str, "\n");
	dict = (t_dict *)malloc(sizeof(t_dict) * (ft_arrlen(dict_lines) + 1));
	if (!dict)
		return (NULL);
	i = 0;
	while (dict_lines[i])
	{
		dict_split = ft_split(dict_lines[i], ":");
		if (ft_arrlen(dict_split) != 2)
			return (ft_parse_error(dict, dict_lines, dict_split, i));
		dict[i].nbr = ft_outspace(dict_split[0]);
		dict[i].word = ft_outspace(dict_split[1]);
		ft_free_split(dict_split);
		i++;
	}
	dict[i].nbr = 0;
	dict[i].word = 0;
	ft_free_split(dict_lines);
	return (dict);
}

t_dict	*ft_read_dict(char *dict_name)
{
	int		fd;
	char	*dict_str;
	t_dict	*dict;

	fd = open(dict_name, O_RDONLY);
	if (fd == -1)
		return (NULL);
	dict_str = ft_read_file(fd);
	close(fd);
	if (!dict_str)
		return (NULL);
	dict = ft_parse_dict(dict_str);
	free(dict_str);
	return (dict);
}
