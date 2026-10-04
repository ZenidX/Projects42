/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strtrim.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/02 02:33:18 by xalara            #+#    #+#             */
/*   Updated: 2026/10/03 22:38:25 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strtrim(char const *s1, char const *set)
{
	char	*p;
	size_t	l;
	size_t	i;
	size_t	j;

	if (ft_strlen(s1) == 0)
	{
		p = (char *) malloc(sizeof(char));
		*p = '\0';
		return (p);
	}
	l = ft_strlen(s1) - 1;
	while (l > 0 && ft_strchr(set, s1[l]))
		l--;
	i = 0;
	while (i < l + 1 && ft_strchr(set, s1[i]))
		i++;
	p = (char *)malloc(sizeof(char) *(l - i + 1));
	j = 0;
	while (j < l - i + 1)
	{
		p[j] = s1[j + i];
		j++;
	}
	p[j] = '\0';
	return (p);
}
