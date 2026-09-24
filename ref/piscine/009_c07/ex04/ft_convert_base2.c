/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_convert_base2.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 12:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/07/23 12:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

int	ft_is_space(char c)
{
	return (c == ' ' || (c >= 9 && c <= 13));
}

int	ft_baselen(char *base)
{
	int	i;

	i = 0;
	while (base[i])
		i++;
	return (i);
}

int	ft_is_valid_base(char *base)
{
	int	i;
	int	j;

	if (ft_baselen(base) < 2)
		return (0);
	i = 0;
	while (base[i])
	{
		if (ft_is_space(base[i]) || base[i] == '+' || base[i] == '-')
			return (0);
		j = i + 1;
		while (base[j])
		{
			if (base[i] == base[j])
				return (0);
			j++;
		}
		i++;
	}
	return (1);
}

int	ft_index_in_base(char *base, char c)
{
	int	i;

	i = 0;
	while (base[i])
	{
		if (base[i] == c)
			return (i);
		i++;
	}
	return (-1);
}

long	ft_atoi_base(char *nbr, char *base)
{
	long	result;
	int		sign;
	int		digit;
	int		len;

	result = 0;
	sign = 1;
	len = ft_baselen(base);
	while (ft_is_space(*nbr))
		nbr++;
	while (*nbr == '+' || *nbr == '-')
	{
		if (*nbr == '-')
			sign = -sign;
		nbr++;
	}
	digit = ft_index_in_base(base, *nbr);
	while (digit >= 0)
	{
		result = result * len + digit;
		nbr++;
		digit = ft_index_in_base(base, *nbr);
	}
	return (result * sign);
}
