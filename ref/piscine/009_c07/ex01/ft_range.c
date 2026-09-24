/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_range.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 12:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/07/23 12:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdlib.h>

int	*ft_range(int min, int max)
{
	int	*arr;
	int	i;
	int	size;

	if (min >= max)
		return (NULL);
	size = max - min;
	arr = (int *)malloc(sizeof(int) * size);
	if (!arr)
		return (NULL);
	i = 0;
	while (i < size)
	{
		arr[i] = min + i;
		i++;
	}
	return (arr);
}
