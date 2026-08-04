/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_sort_params.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <marvin@42.fr>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/30 11:46:09 by xalara            #+#    #+#             */
/*   Updated: 2026/07/30 13:01:43 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

int	ft_strcmp(char *a, char *b)
{
	int	i;

	i = 0;
	while (a[i] && b[i] && a[i] == b[i])
		i++;
	return (a[i] - b[i]);
}

void	ft_swap(char *a, char *b)
{
	char	*s;

	s = a;
	a = b;
	b = s;
}

void	ft_sort(char **tab, int size)
{
	int	i;

	i = 0;
	while (i < size - 1)
	{
		if (ft_strcmp(tab[i], tab[i + 1]) < 0)
		{
			ft_swap(tab[i], tab[i + 1]);
			i = 0;
		}
		else
			i++;
	}
}

void	ft_putstr(char *str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		write(1, &str[i], 1);
		i++;
	}
}

int	main(int argc, char **argv)
{
	int	i;

	argv[0] = NULL;
	ft_sort(argv, argc - 1);
	i = 1;
	while (i < argc)
	{
		ft_putstr(argv[i]);
		write(1, "\n", 1);
		i++;
	}
}
