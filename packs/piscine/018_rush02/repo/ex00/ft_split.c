/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/06 15:21:10 by xalara            #+#    #+#             */
/*   Updated: 2026/08/09 02:29:24 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>

int	ft_in_charset(char c, char *charset)
{
	int	i;

	i = 0;
	if (c == '\0')
		return (1);
	while (charset[i])
	{
		if (c == charset[i])
			return (1);
		i++;
	}
	return (0);
}

int	ft_count_words(char *str, char *charset)
{
	int	c;
	int	f;
	int	i;

	c = 0;
	f = 0;
	i = 0;
	while (1)
	{
		if (!ft_in_charset(str[i], charset) && f == 0)
		{
			f = 1;
			c++;
		}
		else if (ft_in_charset(str[i], charset) && f == 1)
		{
			f = 0;
		}
		if (!str[i])
			break ;
		i++;
	}
	return (c);
}

/*
** Devuelve un bloque nuevo con el trozo de src que va de ini (incluido) a fin
** (excluido). La memoria la reserva aqui dentro: quien llama solo recoge el
** retorno, y es quien tiene que liberarlo.
*/
char	*ft_strifcpy(char *src, int ini, int fin)
{
	char	*dest;
	int		i;

	dest = (char *)malloc(sizeof(char) * (fin - ini + 1));
	if (!dest)
		return (0);
	i = 0;
	while (i < fin - ini)
	{
		dest[i] = src[ini + i];
		i++;
	}
	dest[i] = '\0';
	return (dest);
}

void	ft_add_word(char *str, char **r, char *charset)
{
	int	i[4];

	i[0] = 0;
	i[1] = 0;
	i[2] = 0;
	i[3] = 0;
	while (1)
	{
		if (!ft_in_charset(str[i[0]], charset) && i[1] == 0)
		{
			i[1] = 1;
			i[2] = i[0];
		}
		else if (ft_in_charset(str[i[0]], charset) && i[1] == 1)
		{
			i[1] = 0;
			r[i[3]] = ft_strifcpy(str, i[2], i[0]);
			i[3]++;
		}
		if (!str[i[0]])
			break ;
		i[0]++;
	}
	r[i[3]] = NULL;
}

char	**ft_split(char *str, char *charset)
{
	char	**r;
	int		n_str;

	n_str = ft_count_words(str, charset);
	r = (char **)malloc(sizeof(char *) * (n_str + 1));
	if (!r)
		return (0);
	ft_add_word(str, r, charset);
	return (r);
}
