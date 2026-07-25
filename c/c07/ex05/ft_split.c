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

static int	is_sep(char c, char *charset)
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

static int	count_words(char *str, char *charset)
{
	int	count;
	int	in_word;
	int	i;

	count = 0;
	in_word = 0;
	i = 0;
	while (str[i])
	{
		if (is_sep(str[i], charset))
			in_word = 0;
		else if (!in_word)
		{
			in_word = 1;
			count++;
		}
		i++;
	}
	return (count);
}

static char	*word_dup(char *str, int start, int end)
{
	char	*word;
	int		i;

	word = (char *)malloc(sizeof(char) * (end - start + 1));
	if (!word)
		return (NULL);
	i = 0;
	while (start < end)
	{
		word[i] = str[start];
		i++;
		start++;
	}
	word[i] = '\0';
	return (word);
}

static char	**fill_words(char **result, char *str, char *charset)
{
	int	i;
	int	start;
	int	w;

	i = 0;
	w = 0;
	while (str[i])
	{
		if (!is_sep(str[i], charset))
		{
			start = i;
			while (str[i] && !is_sep(str[i], charset))
				i++;
			result[w] = word_dup(str, start, i);
			w++;
		}
		else
			i++;
	}
	result[w] = NULL;
	return (result);
}

char	**ft_split(char *str, char *charset)
{
	char	**result;

	result = (char **)malloc(sizeof(char *) * (count_words(str, charset) + 1));
	if (!result)
		return (NULL);
	return (fill_words(result, str, charset));
}
