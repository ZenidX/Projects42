/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_sort_params.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 15:28:31 by xalara            #+#    #+#             */
/*   Updated: 2026/09/28 21:39:15 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

void	ft_putchar(char a);

int	ft_strcmp(char *s1, char *s2)
{
	int	i;

	i = 0;
	while (s1[i] && s2[i] && s1[i] == s2[i])
		i++;
	return ((unsigned char) s1[i] - (unsigned char) s2[i]);
}

void	ft_putarraystr(int argn, char **argv)
{
	int	i;
	int	j;

	i = 1;
	while (i < argn)
	{
		j = 0;
		while (argv[i][j])
		{
			ft_putchar(argv[i][j]);
			j++;
		}
		ft_putchar(10);
		i++;
	}
}

int	main(int argn, char **argv)
{
	int		i;
	int		j;
	char	*temp;

	if (argn < 2)
		return (0);
	i = 1;
	while (i < (argn))
	{
		j = i + 1;
		while (j < argn)
		{
			if (ft_strcmp(argv[i], argv[j]) > 0)
			{
				temp = argv[i];
				argv[i] = argv[j];
				argv[j] = temp;
			}
			j++;
		}
		i++;
	}
	ft_putarraystr(argn, argv);
	return (0);
}
