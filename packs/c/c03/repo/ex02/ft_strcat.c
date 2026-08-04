/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strcat.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 12:34:28 by xalara            #+#    #+#             */
/*   Updated: 2026/07/28 13:51:40 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

char	*ft_strcat(char *dest, char *src)
{
	int	i;
	int	j;

	i = 0;
	while (dest[i])
		i++;
	j = 0;
	while (src[j])
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

	dest = "Saludos,                                 ";
	src = "curpo escombro";
	ft_print(ft_strcat(dest, src));
}
*/
