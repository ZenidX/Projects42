/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/06 15:21:10 by xalara            #+#    #+#             */
/*   Updated: 2026/08/10 11:54:25 by xalara           ###   ########.fr       */
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

void	ft_strifcpy(char *dest, char *src, int ini, int fin)
{
	int	i;

	i = 0;
	while (i < fin - ini)
	{
		dest[i] = src[ini + i];
		i++;
	}
	dest[i] = '\0';
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
			r[i[3]] = (char *)malloc(sizeof(char) * (i[0] - i[2] + 1));
			ft_strifcpy(r[i[3]], str, i[2], i[0]);
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
