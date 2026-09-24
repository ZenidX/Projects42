/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   auxiliar_functions.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marchern <marchern@student.42barcelon      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 11:20:28 by marchern          #+#    #+#             */
/*   Updated: 2026/08/09 18:54:55 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include "rush02.h"

int	ft_is_positive(char *str)
{
	int	i;

	i = 0;
	if (!str[i])
		return (0);
	while (str[i])
	{
		if (str[i] >= '0' && str[i] <= '9')
			i++;
		else
			return (0);
	}
	return (1);
}

int	ft_strlen(char *str)
{
	int	len;

	len = 0;
	while (str[len])
		len++;
	return (len);
}

void	ft_putstr(char *str)
{
	write(1, str, ft_strlen(str));
}

int	ft_strcmp(char *s1, char *s2)
{
	int	i;

	i = 0;
	while ((s1[i] != '\0' && s2[i] != '\0') && (s1[i] == s2[i]))
		i++;
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

int	ft_strncmp(char *s1, char *s2, unsigned int n)
{
	unsigned int	i;

	if (n == 0)
		return (0);
	i = 0;
	while ((s1[i] != '\0') && (s1[i] == s2[i]) && (i < (n - 1)))
	{
		i++;
	}
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}
