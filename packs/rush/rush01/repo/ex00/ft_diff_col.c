/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_diff_col.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltejada <ltejada@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/02 18:39:25 by ltejada           #+#    #+#             */
/*   Updated: 2026/08/02 19:00:28 by ltejada          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
int	ft_diff_col(int pos[9][9], int size)
{
	int	i;
	int	j;
	int	z;

	i = 0;
	while (i < size)
	{
		j = 0;
		while (j < size)
		{
			z = i + 1;
			while (z < size)
			{
				if (pos[i][j] == pos[z][j] && i != z)
					return (0);
				z++;
			}
			j++;
		}
		i++;
	}
	return (1);
}
