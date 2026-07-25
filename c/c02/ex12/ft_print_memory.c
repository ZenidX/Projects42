/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_memory.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 12:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/07/23 12:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

static void	ft_addr(unsigned long n)
{
	char	*h;
	char	buf[16];
	int		i;

	h = "0123456789abcdef";
	i = 16;
	while (i > 0)
	{
		i--;
		buf[i] = h[n % 16];
		n = n / 16;
	}
	write(1, buf, 16);
	write(1, ": ", 2);
}

static void	ft_hexline(unsigned char *p, unsigned int start, unsigned int size)
{
	char	*h;
	int		j;

	h = "0123456789abcdef";
	j = 0;
	while (j < 16)
	{
		if (start + j < size)
		{
			write(1, &h[p[start + j] / 16], 1);
			write(1, &h[p[start + j] % 16], 1);
		}
		else
			write(1, "  ", 2);
		if (j % 2 == 1)
			write(1, " ", 1);
		j++;
	}
}

static void	ft_ascii(unsigned char *p, unsigned int start, unsigned int size)
{
	unsigned char	c;
	int				j;

	j = 0;
	while (j < 16 && start + j < size)
	{
		c = p[start + j];
		if (c >= 32 && c <= 126)
			write(1, &c, 1);
		else
			write(1, ".", 1);
		j++;
	}
	write(1, "\n", 1);
}

void	*ft_print_memory(void *addr, unsigned int size)
{
	unsigned char	*p;
	unsigned int	start;

	if (size == 0)
		return (addr);
	p = (unsigned char *)addr;
	start = 0;
	while (start < size)
	{
		ft_addr((unsigned long)addr + start);
		ft_hexline(p, start, size);
		ft_ascii(p, start, size);
		start = start + 16;
	}
	return (addr);
}
