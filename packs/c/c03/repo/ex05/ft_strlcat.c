/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strlcat.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 13:27:50 by xalara            #+#    #+#             */
/*   Updated: 2026/08/04 01:06:57 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

unsigned int	ft_strlcat(char *dest, char *src, unsigned int size)
{
	unsigned int	i;
	unsigned int	j;
	unsigned int	k;

	i = 0;
	while (i < size && dest[i])
		i++;
	j = 0;
	while (src[j])
		j++;
	if (size == i)
		return (size + j);
	k = 0;
	while (src[k] && i + k + 1 < size)
	{
		dest[i + k] = src[k];
		j++;
	}
	dest[i + k] = '\0';
	return (i + j);
}
/*
void	ft_putstr(char *s)
{
	int	i;

	i = 0;
	while (s[i])
	{
		write(1, &s[i], 1);
		i++;
	}
}

int	main(void)
{
	char	dest[100];
	char	*main;
	char	*src;

	main = "Saludos, desgraciado!      ";
	ft_strlcat(dest, main, 0);
	src = "buen hombre.";
	ft_strlcat(dest, src, 9);
	ft_putstr(dest);
}
*/
