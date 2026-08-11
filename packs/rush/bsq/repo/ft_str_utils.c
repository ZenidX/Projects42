/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_str_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/11 02:07:58 by xalara            #+#    #+#             */
/*   Updated: 2026/08/11 02:20:12 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>

//Se usa...
int	ft_strlen(char *str)
{
	int	len;

	len = 0;
	while (str[len])
		len++;
	return (len);
}
//Se usa...
void ft_putstr(char *str)
{
	write(1, str, ft_sstrlen(str));
}

/*
** Devuelve un bloque NUEVO con s1 y s2 pegados, y CONSUME s1: lo libera antes
** de volver. Pensada para acumular en bucle (s = ft_strcat(s, trozo)), donde
** el bloque anterior ya no sirve. Por eso s1 tiene que venir del heap: nunca
** le pases un literal ni un array de la pila. s2 solo se lee, ese no se toca.
** Si el malloc falla devuelve 0 y s1 se pierde sin liberar.
*/
char *ft_strcat(char *s1, char *s2)
{
	char	*r;
	int		i;
	int		j;

	r = (char *)malloc(sizeof(char) *(ft_strlen(s1) + ft_strlen(s2) +1));
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

//No se usa...
int	ft_strncmp(char *s1, char *s2, unsigned int n)
{
	unsigned int	i;

	if (n == 0)
		return (0);
	i = 0;
	while (i < n - 1 && s1[i] && s2[i] && s1[i] == s2[i])
		i++;
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

