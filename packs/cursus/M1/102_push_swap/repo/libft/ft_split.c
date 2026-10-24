/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/02 03:13:33 by xalara            #+#    #+#             */
/*   Updated: 2026/10/04 17:35:40 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	ft_count_words(char const *s, char c)
{
	int	r;
	int	f;
	int	i;

	r = 0;
	f = 0;
	i = 0;
	while (1)
	{
		if (s[i] && s[i] != c && f == 0)
		{
			f = 1;
			r++;
		}
		else if ((s[i] == c || !s[i]) && f == 1)
		{
			f = 0;
		}
		if (!s[i])
			break ;
		i++;
	}
	return (r);
}

static int	ft_free_split(char **r)
{
	int	i;

	i = 0;
	while (r[i])
	{
		free(r[i]);
		i++;
	}
	free(r);
	return (0);
}

static char	*ft_word_dup(char const *src, int ini, int fin)
{
	char	*dest;
	int		i;

	dest = malloc(fin - ini + 1);
	if (!dest)
		return (NULL);
	i = 0;
	while (i < fin - ini)
	{
		dest[i] = src[ini + i];
		i++;
	}
	dest[i] = '\0';
	return (dest);
}

static int	ft_add_word(char const *s, char **r, char c)
{
	int	i[4];

	ft_bzero(i, sizeof(i));
	while (1)
	{
		if (s[i[0]] && s[i[0]] != c && i[1] == 0)
		{
			i[1] = 1;
			i[2] = i[0];
		}
		else if ((s[i[0]] == c || !s[i[0]]) && i[1] == 1)
		{
			i[1] = 0;
			r[i[3]] = ft_word_dup(s, i[2], i[0]);
			if (!r[i[3]])
				return (ft_free_split(r));
			i[3]++;
		}
		if (!s[i[0]])
			break ;
		i[0]++;
	}
	r[i[3]] = NULL;
	return (1);
}

char	**ft_split(char const *s, char c)
{
	char	**r;
	size_t	n_str;

	n_str = ft_count_words(s, c);
	r = (char **)malloc(sizeof(char *) * (n_str + 1));
	if (!r)
		return (0);
	if (ft_add_word(s, r, c))
		return (r);
	else
		return (NULL);
}
