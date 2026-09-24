/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_strstr.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/27 12:55:25 by xalara            #+#    #+#             */
/*   Updated: 2026/07/31 11:31:48 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

char	*ft_strstr(char *str, char *to_find)
{
	int	i;
	int	j;

	if (!to_find[0])
		return (str);
	i = 0;
	while (str[i])
	{
		j = 0;
		while (to_find[j] && str[i + j] && str[i + j] == to_find[j])
		{
			j++;
		}
		if (!to_find[j])
			return (&str[i]);
		i++;
	}
	return ((void *)0);
}

void	ft_strcpy(char *d, char *s)
{
	int	i;

	i = 0;
	while (s[i])
	{
		d[i] = s[i];
		i++;
	}
	d[i] = '\0';
}

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
	char	*a;
	char	*b;
	char	*r;

	a = "Hola";
	b = "e";
	r = ft_strstr(a, b);
	if (r)
		ft_putstr(r);
	else
		ft_putstr("(null)");
	ft_putstr("\n");
	return (0);
}
