/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_auxiliar.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/11 02:20:45 by xalara            #+#    #+#             */
/*   Updated: 2026/08/11 02:28:20 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>
#include "bsq.h"

/* Devuelve el numero de elementoss hasta el centinela NULL al final*/
int	ft_arrlen(char **arr)
{
	int	i;

	i = 0;
	while (arr[i])
		i++;
	return (i);
}

/* Libera un array de (char *) con centinela NULL al final*/
void ft_free_split(char **arr)
{
	int	i;

	i = 0;
	while (arr[i])
	{
		free(arr[i]);
		i++;
	}
	free(arr);
}

/* Libera el mapa de la estructura mapa como split y la misma estructura*/
void ft_free_map(t_map map)
{
	ft_free_split(map.map);
	free(map);
}
