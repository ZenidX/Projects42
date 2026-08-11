/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   rush02.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/08 13:24:26 by xalara            #+#    #+#             */
/*   Updated: 2026/08/09 19:48:10 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include "rush02.h"

/* Con dos argumentos el primero es el diccionario y el segundo el numero. */
void	get_args(int argc, char **argv, char **dict_name, char **num)
{
	if (argc == 3)
	{
		*dict_name = argv[1];
		*num = argv[2];
	}
	else
	{
		*dict_name = "numbers.dict";
		*num = argv[1];
	}
}

/* El numero es "-" a secas: los numeros vienen por la entrada estandar. */
int	is_stdin(char *num)
{
	return (num[0] == '-' && !num[1]);
}

/*
** Lee toda la entrada estandar y convierte una linea por numero. Reaprovecha
** ft_read_file, que ya trabaja con un descriptor cualquiera, y ft_split para
** trocear por saltos de linea. Las lineas vacias las descarta ft_split sola.
*/
void	read_stdin(t_dict *dict)
{
	char	**lines;
	char	*content;
	int		i;

	content = ft_read_file(0);
	if (!content)
		return ;
	lines = ft_split(content, "\n");
	free(content);
	i = 0;
	while (lines[i])
	{
		if (ft_is_positive(lines[i]))
			ft_write_nbr(lines[i], dict);
		else
			ft_putstr("Error\n");
		i++;
	}
	ft_free_split(lines);
}

int	main(int argc, char **argv)
{
	char	*num;
	char	*dict_name;
	t_dict	*dict;

	if (argc != 2 && argc != 3)
		return (0);
	get_args(argc, argv, &dict_name, &num);
	if (!is_stdin(num) && !ft_is_positive(num))
	{
		ft_putstr("Error\n");
		return (0);
	}
	dict = ft_read_dict(dict_name);
	if (dict == NULL)
	{
		ft_putstr("Dict Error\n");
		return (0);
	}
	if (is_stdin(num))
		read_stdin(dict);
	else
		ft_write_nbr(num, dict);
	ft_free_dict(dict);
	return (0);
}
