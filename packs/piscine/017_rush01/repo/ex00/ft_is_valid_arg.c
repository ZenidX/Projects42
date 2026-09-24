/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_is_valid_arg.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ltejada <ltejada@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/01 11:14:31 by ltejada           #+#    #+#             */
/*   Updated: 2026/08/02 17:19:17 by ltejada          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
int	ft_is_valid_arg(char *str)
{
	int	length;
	int	check;

	length = 0;
	check = 0;
	while (*str)
	{
		if (('1' <= *str && *str <= '9') && !check)
		{
			length++;
			check = 1;
			str++;
		}
		else if (*str == ' ' && check)
		{
			check = 0;
			str++;
		}
		else
			break ;
	}
	if (length % 4 == 0)
		return (length / 4);
	return (0);
}
