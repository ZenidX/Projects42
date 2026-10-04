/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/02 02:33:18 by xalara            #+#    #+#             */
/*   Updated: 2026/10/04 04:06:30 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

static int	ft_trim_r(char const *s1, char const *set)
{
	int	l;

	l = ft_strlen(s1) - 1;
	while (l > 0 && ft_strchr(set, s1[l]))
		l--;
	return (l);
}

static int	ft_trim_l(char const *s1, char const *set, int l)
{
	int	i;

	i = 0;
	while (i < l + 1 && ft_strchr(set, s1[i]))
		i++;
	return (i);
}

char	*ft_strtrim(char const *s1, char const *set)
{
	char	*p;
	size_t	l;
	size_t	i;

	if (!s1)
		return (NULL);
	if (ft_strlen(s1) == 0)
	{
		p = (char *) malloc(sizeof(char));
		if (!p)
			return (NULL);
		*p = '\0';
		return (p);
	}
	l = ft_trim_r(s1, set);
	i = ft_trim_l(s1, set, l);
	return (ft_substr(s1, i, l - i + 1));
}
