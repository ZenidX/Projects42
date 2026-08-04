/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strncat.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 12:34:28 by xalara            #+#    #+#             */
/*   Updated: 2026/07/29 15:10:50 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

char	*ft_strncat(char *dest, char *src, unsigned int n)
{
	unsigned int	i;
	unsigned int	j;

	i = 0;
	while (dest[i])
		i++;
	j = 0;
	while (src[j] && j < n)
	{
		dest[i + j] = src[j];
		j++;
	}
	dest[i + j] = '\0';
	return (dest);
}
/*
void	ft_print(char *s)
{
	int	i;

	i = 0;
	while (s[i])
		write(1, &s[i], 1);
}

int	main(void)
{
	char	*dest;
	char	*src;

	dest = "Saludos, ";
	src = "curpo escombro";
	ft_print(ft_strcat(dest, src));
}
*/
