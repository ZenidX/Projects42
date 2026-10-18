/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strrchr.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/30 12:15:31 by xalara            #+#    #+#             */
/*   Updated: 2026/10/01 23:59:29 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

char	*ft_strrchr(const char *s, int c)
{
	char	*p;

	p = ((char *) s) + ft_strlen(s);
	while (*p != (char)c && p != s)
		p--;
	if (*p != (char) c && p == s)
		return (NULL);
	else
		return (p);
}
