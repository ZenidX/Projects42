/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_hex.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/17 09:49:34 by xalara            #+#    #+#             */
/*   Updated: 2026/10/18 13:32:16 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

static char *ft_itoa_base(unsigned long n, int base_len, char *base, char *r)
{
	char	*q;

	if (base_len == 0)
		return (NULL);
	if (n / base_len > 0)
		r = ft_itoa_base(n / base_len, base_len, base, r);
	else if (n == base_len)
	{
		
	}
	else
	{
		r = ft_strdup(r, ft_strlen(r) + 1); 
		r = ft_strncat(r,b[n % base_len], ft_strlen(r) + 1);
	}
	return (r);
}

int	ft_print_hex(unsigned long n, int upper)
{
	char *str;
	char *hex;

	if (upper)
		hex = "0123456789abcdef";
	else
		hex = "0123456789ABCDEF";
	str = ft_itoa_base(n, ft_strlen(hex), hex);
	ft_print_str(str);
	free(str);
	return (0);
}

/* TODO %p: "0x" + hex en minusculas; con NULL, "(nil)" como glibc. */
int	ft_print_ptr(void *p)
{
	(void)p;
	return (0);
}
