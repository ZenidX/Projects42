/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_chars.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/17 09:49:34 by xalara            #+#    #+#             */
/*   Updated: 2026/10/18 12:45:58 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

ssize_t	ft_print_char(int c)
{
	unsigned char	chr;

	chr = (unsigned char) c;
	return (write(1, &chr, 1));
}

ssize_t	ft_print_str(char *str)
{
	if (!str)
		str = "(null)";
	return (write(1, str, ft_strlen(str));
}
