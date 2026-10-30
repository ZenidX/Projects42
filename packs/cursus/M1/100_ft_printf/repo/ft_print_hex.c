/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_hex.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/17 09:49:34 by xalara            #+#    #+#             */
/*   Updated: 2026/10/18 15:38:55 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_print_hex(unsigned long n, int upper)
{
	char	*hex;

	if (upper)
		hex = "0123456789ABCDEF";
	else
		hex = "0123456789abcdef";
	return (ft_print_free(ft_itoa_base(n, hex, 16, 0)));
}

/* "0x" + la direccion en hex; NULL sale "(nil)", como en glibc. */
int	ft_print_ptr(void *p)
{
	int	ret;

	if (!p)
		return (ft_print_str("(nil)"));
	if (ft_print_str("0x") < 0)
		return (-1);
	ret = ft_print_hex((unsigned long)p, 0);
	if (ret < 0)
		return (-1);
	return (ret + 2);
}
