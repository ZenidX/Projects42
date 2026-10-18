/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_printf.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/17 09:49:34 by xalara            #+#    #+#             */
/*   Updated: 2026/10/17 09:49:34 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

/* Una conversion: devuelve lo que ha escrito, o -1 si write ha fallado. */
static int	ft_conv(char c, va_list *ap)
{
	if (c == 'c')
		return (ft_print_char(va_arg(*ap, int)));
	if (c == 's')
		return (ft_print_str(va_arg(*ap, char *)));
	if (c == 'p')
		return (ft_print_ptr(va_arg(*ap, void *)));
	if (c == 'd' || c == 'i')
		return (ft_print_nbr(va_arg(*ap, int)));
	if (c == 'u')
		return (ft_print_unsigned(va_arg(*ap, unsigned int)));
	if (c == 'x')
		return (ft_print_hex(va_arg(*ap, unsigned int), 0));
	if (c == 'X')
		return (ft_print_hex(va_arg(*ap, unsigned int), 1));
	if (c == '%')
		return (ft_print_char('%'));
	return (-1);
}

int	ft_printf(const char *format, ...)
{
	va_list	ap;
	int		count;
	int		ret;

	if (!format)
		return (-1);
	va_start(ap, format);
	count = 0;
	while (*format)
	{
		if (*format == '%' && *(format + 1))
			ret = ft_conv(*(++format), &ap);
		else
			ret = ft_print_char(*format);
		if (ret < 0)
		{
			va_end(ap);
			return (-1);
		}
		count += ret;
		format++;
	}
	va_end(ap);
	return (count);
}
