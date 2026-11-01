/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort_adaptive.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/25 01:38:51 by xalara            #+#    #+#             */
/*   Updated: 2026/11/01 13:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** Estrategia por defecto (sin flag): mira el regimen de desorden (ver
** disorder_regime en index.c) y usa el metodo con la complejidad que pide
** el enunciado para ese regimen: sort_low (sort_low.c) para el bajo, y
** sort_medium y sort_complex para el medio y el alto.
*/
void	sort_adaptive(t_ps *ps)
{
	int	regime;

	regime = disorder_regime(ps);
	if (regime == 0)
		sort_low(ps);
	else if (regime == 1)
		sort_medium(ps);
	else
		sort_complex(ps);
}
