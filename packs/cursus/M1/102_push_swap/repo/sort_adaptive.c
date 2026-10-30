/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sort_adaptive.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/25 01:38:51 by xalara            #+#    #+#             */
/*   Updated: 2026/10/25 01:38:51 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** POR HACER: metodo O(n) para cuando hay poco desorden (d < 0.2), en el que
** casi todo ya esta en su sitio y basta con colocar los pocos que no.
*/
static void	sort_low(t_ps *ps)
{
	(void)ps;
}

/*
** Estrategia por defecto (sin flag): mira el regimen de desorden (ver
** disorder_regime en index.c) y usa el metodo con la complejidad que pide
** el enunciado para ese regimen. Los regimenes medio y alto reutilizan
** sort_medium y sort_complex.
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
