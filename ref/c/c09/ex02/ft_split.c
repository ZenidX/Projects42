/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_split.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 12:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/07/23 12:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>

static int	ft_is_sep(char c, char *charset)
{
	int	i;

	i = 0;
	while (charset[i])
	{
		if (charset[i] == c)
			return (1);
		i++;
	}
	return (0);
}

static int	ft_count_words(char *str, char *charset)
{
	int	i;
	int	words;

	i = 0;
	words = 0;
	while (str[i])
	{
		while (str[i] && ft_is_sep(str[i], charset))
			i++;
		if (str[i] && !ft_is_sep(str[i], charset))
			words++;
		while (str[i] && !ft_is_sep(str[i], charset))
			i++;
	}
	return (words);
}

static char	*ft_dup_word(char *str, char *charset)
{
	int		len;
	int		i;
	char	*word;

	len = 0;
	while (str[len] && !ft_is_sep(str[len], charset))
		len++;
	word = (char *)malloc(sizeof(char) * (len + 1));
	if (!word)
		return (0);
	i = 0;
	while (i < len)
	{
		word[i] = str[i];
		i++;
	}
	word[i] = '\0';
	return (word);
}

static int	ft_fill(char **tab, char *str, char *charset)
{
	int	i;
	int	w;

	i = 0;
	w = 0;
	while (str[i])
	{
		while (str[i] && ft_is_sep(str[i], charset))
			i++;
		if (str[i])
		{
			tab[w] = ft_dup_word(str + i, charset);
			if (!tab[w])
				return (0);
			w++;
		}
		while (str[i] && !ft_is_sep(str[i], charset))
			i++;
	}
	tab[w] = 0;
	return (1);
}

char	**ft_split(char *str, char *charset)
{
	char	**tab;

	tab = (char **)malloc(sizeof(char *) * (ft_count_words(str, charset) + 1));
	if (!tab)
		return (0);
	if (!ft_fill(tab, str, charset))
	{
		free(tab);
		return (0);
	}
	return (tab);
}
