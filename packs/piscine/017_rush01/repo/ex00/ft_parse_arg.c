/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_parse_arg.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltejada <ltejada@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/01 11:48:53 by ltejada           #+#    #+#             */
/*   Updated: 2026/08/02 17:24:22 by ltejada          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
int	ft_parse_arg(int inp[4][9], char *str, int size)
{
	int	i;
	int	j;

	i = 0;
	j = 0;
	while (*str)
	{
		if (j >= size)
		{
			j = 0;
			i++;
		}
		if ('1' <= *str && *str <= ('0' + size))
		{
			inp[i][j] = *str - 0x30;
			j++;
		}
		else if (*str == ' ')
			str = str + 0;
		else
			return (0);
		str++;
	}
	return (1);
}
