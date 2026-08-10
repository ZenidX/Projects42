/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_convert_base.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/04 02:34:11 by xalara            #+#    #+#             */
/*   Updated: 2026/08/06 15:15:58 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <stdlib.h>

int		ft_strlen(char *s);

char	*ft_char_init(char *s, int size);

int	ft_in_base(char c, char *str)
{
	int	i;

	i = 0;
	while (str[i] && str[i] != c)
	{
		i++;
	}
	if (str[i])
		return (i);
	else
		return (-1);
}

int	ft_base_ok(char *b)
{
	int	i;
	int	j;

	if (!b[0] || !b[1])
		return (0);
	i = 0;
	while (b[i])
	{
		if (b[i] == ' ' || (b[i] >= 9 && b[i] <= 13)
			|| b[i] == '+' || b[i] == '-')
			return (0);
		j = 0;
		while (j < i)
		{
			if (b[i] == b[j])
				return (0);
			j++;
		}
		i++;
	}
	return (i);
}

int	ft_atoi_base(char *c, char *base, int b)
{
	int	r;
	int	s;
	int	i;

	r = 0;
	s = 1;
	i = 0;
	while (c[i] == ' ' || (c[i] >= 9 && c[i] <= 13))
		i++;
	while (c[i] == '+' || c[i] == '-')
	{
		if (c[i] == '-')
			s = -s;
		i++;
	}
	while (ft_in_base(c[i], base) >= 0)
	{
		r = r * b + s * ft_in_base(c[i], base);
		i++;
	}
	return (r);
}

char	*ft_itoa_base(int n, char *base, int b, int d)
{
	char	*r;
	int		dd;
	int		neg;

	neg = (n < 0);
	if (n / b != 0)
		r = ft_itoa_base(n / b, base, b, d + 1);
	else
	{
		r = (char *)malloc(sizeof(char) * (d + 2 + neg));
		r = ft_char_init(r, d + 1 + neg);
	}
	if (!r)
		return (NULL);
	dd = ft_strlen(r);
	if (neg)
	{
		r[0] = '-';
		r[dd - 1 - d] = base[-(n % b)];
	}
	else
		r[dd - 1 - d] = base[n % b];
	return (r);
}

char	*ft_convert_base(char *nbr, char *base_from, char *base_to)
{
	int		bf;
	int		bt;
	int		n;
	char	*r;

	bf = ft_base_ok(base_from);
	bt = ft_base_ok(base_to);
	if (!bf || !bt)
		return (NULL);
	n = ft_atoi_base(nbr, base_from, bf);
	r = ft_itoa_base(n, base_to, bt, 0);
	return (r);
}
