/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bench.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/25 01:38:51 by xalara            #+#    #+#             */
/*   Updated: 2026/10/25 01:38:51 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** Modo --bench. Todo sale por stderr (fd 2): stdout queda solo con las
** operaciones, que es lo que lee el checker, y asi --bench no lo estropea.
*/

/*
** Imprime el desorden como porcentaje con dos decimales sin usar float.
** p es el desorden en centesimas de porcentaje (1 = 0.01 %):
**     p = mistakes * 10000 / pairs, redondeado al mas cercano
** (el + pairs / 2 hace que la division entera redondee en vez de truncar).
** Despues: p / 100 es la parte entera y p % 100 los dos decimales.
**     1 de 3  ->  p = (10000 + 1) / 3 = 3333  ->  "33.33%"
*/
static void	put_disorder(t_ps *ps)
{
	long	p;

	p = 0;
	if (ps->pairs)
		p = (ps->mistakes * 10000 + ps->pairs / 2) / ps->pairs;
	ft_putstr_fd("[bench] disorder:  ", 2);
	ft_putnbr_fd(p / 100, 2);
	ft_putchar_fd('.', 2);
	ft_putchar_fd('0' + p % 100 / 10, 2);
	ft_putchar_fd('0' + p % 10, 2);
	ft_putendl_fd("%", 2);
}

/*
** Nombre de la estrategia y su clase de complejidad. Con adaptive, la clase
** depende del regimen que eligio sort_adaptive, asi que se vuelve a
** calcular igual que alli.
*/
static void	put_strategy(t_ps *ps)
{
	int	regime;

	ft_putstr_fd("[bench] strategy:  ", 2);
	regime = disorder_regime(ps);
	if (ps->strategy == SIMPLE)
		ft_putendl_fd("Simple / O(n²)", 2);
	else if (ps->strategy == MEDIUM)
		ft_putendl_fd("Medium / O(n√n)", 2);
	else if (ps->strategy == COMPLEX)
		ft_putendl_fd("Complex / O(n log n)", 2);
	else if (regime == 0)
		ft_putendl_fd("Adaptive / O(n)", 2);
	else if (regime == 1)
		ft_putendl_fd("Adaptive / O(n√n)", 2);
	else
		ft_putendl_fd("Adaptive / O(n log n)", 2);
}

/* "nombre:  n" seguido de end (separador o salto de linea). */
static void	put_count(char *name, int n, char *end)
{
	ft_putstr_fd(name, 2);
	ft_putstr_fd(":  ", 2);
	ft_putnbr_fd(n, 2);
	ft_putstr_fd(end, 2);
}

/*
** El informe entero: desorden, estrategia, total de operaciones (la suma
** de los 11 contadores) y el desglose. Un ejemplo con 1 3 2 (las cifras
** de operaciones son inventadas):
**
**     [bench] disorder:  33.33%
**     [bench] strategy:  Adaptive / O(n√n)
**     [bench] total_ops:  5
**     [bench] sa:  1  sb:  0  ss:  0  pa:  2  pb:  2
**     [bench] ra:  0  rb:  0  rr:  0  rra:  0  rrb:  0  rrr:  0
*/
void	print_bench(t_ps *ps)
{
	int	total;
	int	i;

	total = 0;
	i = 0;
	while (i < 11)
		total += ps->count[i++];
	put_disorder(ps);
	put_strategy(ps);
	ft_putstr_fd("[bench] total_ops:  ", 2);
	ft_putnbr_fd(total, 2);
	ft_putstr_fd("\n[bench] ", 2);
	put_count("sa", ps->count[SA], "  ");
	put_count("sb", ps->count[SB], "  ");
	put_count("ss", ps->count[SS], "  ");
	put_count("pa", ps->count[PA], "  ");
	put_count("pb", ps->count[PB], "\n[bench] ");
	put_count("ra", ps->count[RA], "  ");
	put_count("rb", ps->count[RB], "  ");
	put_count("rr", ps->count[RR], "  ");
	put_count("rra", ps->count[RRA], "  ");
	put_count("rrb", ps->count[RRB], "  ");
	put_count("rrr", ps->count[RRR], "\n");
}
