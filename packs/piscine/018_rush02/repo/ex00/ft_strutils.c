/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strutils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 18:30:00 by xalara            #+#    #+#             */
/*   Updated: 2026/08/09 18:30:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>
#include "rush02.h"

/*
** Devuelve un bloque NUEVO con s1 y s2 pegados, y CONSUME s1: lo libera antes
** de volver. Pensada para acumular en bucle (s = ft_strcat(s, trozo)), donde
** el bloque anterior ya no sirve. Por eso s1 tiene que venir del heap: nunca
** le pases un literal ni un array de la pila. s2 solo se lee, ese no se toca.
** Si el malloc falla devuelve 0 y s1 se pierde sin liberar.
*/
char	*ft_strcat(char *s1, char *s2)
{
	char	*r;
	int		i;
	int		j;

	r = (char *)malloc(sizeof(char) * (ft_strlen(s1) + ft_strlen(s2) + 1));
	if (!r)
		return (0);
	i = 0;
	while (s1[i])
	{
		r[i] = s1[i];
		i++;
	}
	j = 0;
	while (s2[j])
	{
		r[i] = s2[j];
		i++;
		j++;
	}
	r[i] = '\0';
	free(s1);
	return (r);
}

int	ft_arrlen(char **arr)
{
	int	i;

	i = 0;
	while (arr[i])
		i++;
	return (i);
}

void	ft_free_split(char **arr)
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

/* Recorre hasta la entrada centinela (nbr a 0) que deja ft_parse_dict. */
void	ft_free_dict(t_dict *dict)
{
	int	i;

	i = 0;
	while (dict[i].nbr)
	{
		free(dict[i].nbr);
		free(dict[i].word);
		i++;
	}
	free(dict);
}
