/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strncmp.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 11:00:40 by xalara            #+#    #+#             */
/*   Updated: 2026/08/03 23:12:30 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

int	ft_strncmp(char *s1, char *s2, unsigned int n)
{
	unsigned int	i;

	if (n == 0)
		return (0);
	i = 0;
	while (i < n - 1 && s1[i] && s2[i] && s1[i] == s2[i])
		i++;
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}
/*
void	print_int(int i)
{
	char	d;

	if (i < 0)
	{
		write(1, "-", 1);
		i = -i;
	}
	if (i / 10 > 0)
		print_int(i / 10);
	d = i % 10 + '0';
	write(1, &d, 1);
}

int	main(void)
{
	char	*s1;
	char	*s2;

	s1 = "Saludos";
	s2 = "Salux";
	print_int(ft_strncmp(s1, s2, 5));
	print_int(ft_strncmp(s2, s1, 5));
	print_int(ft_strncmp(s2, s2, 5));
}
*/
