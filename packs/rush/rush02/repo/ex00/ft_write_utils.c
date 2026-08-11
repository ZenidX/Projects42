/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_write_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 21:10:00 by xalara            #+#    #+#             */
/*   Updated: 2026/08/09 21:10:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rush02.h"

/* Busca una clave en el diccionario. 0 si no esta: eso es un Dict Error. */
char	*find_word(t_dict *dict, char *key)
{
	int	i;

	i = 0;
	while (dict[i].nbr)
	{
		if (ft_strcmp(dict[i].nbr, key) == 0)
			return (dict[i].word);
		i++;
	}
	return (0);
}

/* Escribe en buf la clave de un numero de 0 a 999. */
char	*ft_itokey(char *buf, int n)
{
	int	i;

	i = 0;
	if (n >= 100)
	{
		buf[i] = '0' + n / 100;
		i++;
	}
	if (n >= 10)
	{
		buf[i] = '0' + (n / 10) % 10;
		i++;
	}
	buf[i] = '0' + n % 10;
	buf[i + 1] = '\0';
	return (buf);
}

/* Escribe en buf la clave de la escala 10^exp: un 1 y exp ceros detras. */
char	*scale_key(char *buf, int exp)
{
	int	i;

	buf[0] = '1';
	i = 1;
	while (i <= exp)
	{
		buf[i] = '0';
		i++;
	}
	buf[i] = '\0';
	return (buf);
}

/*
** Las tres cifras que ocupan las posiciones 10^exp .. 10^(exp+2) del numero.
** El grupo de mas a la izquierda puede tener 1 o 2 cifras en vez de 3, y de
** ahi el ajuste de cuando start se sale por delante de la cadena.
*/
int	group_value(char *nbr, int len, int exp)
{
	int	start;
	int	size;
	int	v;
	int	i;

	start = len - exp - 3;
	size = 3;
	if (start < 0)
	{
		size = start + 3;
		start = 0;
	}
	v = 0;
	i = 0;
	while (i < size)
	{
		v = v * 10 + (nbr[start + i] - '0');
		i++;
	}
	return (v);
}
