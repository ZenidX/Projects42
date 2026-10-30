/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_chars.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/17 09:49:34 by xalara            #+#    #+#             */
/*   Updated: 2026/10/18 16:05:52 by xalara           ###   ########.fr       */
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
	return (write(1, str, ft_strlen(str)));
}

int	ft_print_free(char *str)
{
	int	ret;

	if (!str)
		return (-1);
	ret = ft_print_str(str);
	free(str);
	return (ret);
}
