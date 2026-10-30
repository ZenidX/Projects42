/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_print_nbrs.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/17 09:49:34 by xalara            #+#    #+#             */
/*   Updated: 2026/10/18 15:46:55 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_printf.h"

int	ft_print_nbr(int n)
{
	return (ft_print_free(ft_itoa(n)));
}

int	ft_print_unsigned(unsigned int n)
{
	return (ft_print_free(ft_itoa_base(n, "0123456789", 10, 0)));
}
