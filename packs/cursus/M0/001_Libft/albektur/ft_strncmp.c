/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strncmp.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: besaipid <besaipid@student.42barcelon      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/21 18:24:06 by besaipid          #+#    #+#             */
/*   Updated: 2026/09/24 03:28:17 by besaipid         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

int	ft_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t	j;

	j = 0;
	while (j < n && (s1[j] || s2[j]))
	{
		if ((unsigned char)s1[j] != (unsigned char)s2[j])
			return ((unsigned char)s1[j] - (unsigned char)s2[j]);
		j++;
	}
	return (0);
}
