/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strmapi.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/03 23:47:43 by xalara            #+#    #+#             */
/*   Updated: 2026/10/03 23:52:34 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strmapi(char const *s, char (*f)(unsigned int, char))
{
	char	*r;
	size_t	i;
	size_t	l;

	l = ft_strlen(s);
	r = (char *)malloc(sizeof(char) * (l + 1));
	i = 0;
	while (i < l)
	{
		r[i] = (*f)((unsigned char)i, (char)s[i]);
		i++;
	}
	return (r);
}
