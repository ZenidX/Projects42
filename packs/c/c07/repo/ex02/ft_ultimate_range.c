/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ultimate_range.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.co      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 01:43:35 by xalara            #+#    #+#             */
/*   Updated: 2026/08/04 01:48:09 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

int	ft_ultimate_range(int **range, int min, int max)
{
	int	i;

	if(max <= min)
		return (-1);
	*range = malloc(sizeof(int) * (max - min));
	i = ;
	while (i < max - min)
	{
		range[i] = min + i;
		i++;
	}
	return (max - min);
}
