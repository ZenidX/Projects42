/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   push_swap.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/25 01:38:51 by xalara            #+#    #+#             */
/*   Updated: 2026/10/25 01:38:51 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/* Libera lo que devuelve ft_split: cada palabra y luego el array. */
void	free_split(char **words)
{
	int	i;

	i = 0;
	while (words[i])
		free(words[i++]);
	free(words);
}

/*
** Libera las dos pilas: los nodos y el int* de cada uno (por eso se pasa
** free como funcion para el content).
*/
void	free_ps(t_ps *ps)
{
	ft_lstclear(&ps->a, free);
	ft_lstclear(&ps->b, free);
}

/*
** Cualquier entrada invalida acaba aqui: el enunciado pide "Error\n" por
** stderr. Se libera antes de salir para no dejar leaks.
*/
void	error_exit(t_ps *ps)
{
	free_ps(ps);
	write(2, "Error\n", 6);
	exit(1);
}

/*
** Ejecuta la estrategia forzada por flag o, si no se paso ninguno, la
** adaptativa (que elige por su cuenta segun el desorden).
*/
void	run_strategy(t_ps *ps)
{
	if (ps->strategy == SIMPLE)
		sort_simple(ps);
	else if (ps->strategy == MEDIUM)
		sort_medium(ps);
	else if (ps->strategy == COMPLEX)
		sort_complex(ps);
	else
		sort_adaptive(ps);
}

/*
** El flujo completo:
**
**   1. Sin argumentos no se hace nada (no es un error).
**   2. parse_args: argumentos -> pila a, ya normalizada a rangos 0..n-1.
**      Si algo falla -> Error.
**   3. Si solo habia flags (pila vacia), no hay nada que ordenar.
**   4. compute_disorder mide el desorden ANTES de mover nada, como pide el
**      enunciado.
**   5. Si ya esta ordenada (mistakes == 0) no se imprime ninguna operacion.
**   6. --bench imprime las estadisticas por stderr al final.
*/
int	main(int argc, char **argv)
{
	t_ps	ps;

	if (argc < 2)
		return (0);
	ft_bzero(&ps, sizeof(t_ps));
	ps.strategy = ADAPTIVE;
	if (!parse_args(&ps, argc - 1, argv + 1))
		error_exit(&ps);
	if (!ps.a)
		return (0);
	compute_disorder(&ps);
	if (ps.mistakes > 0)
		run_strategy(&ps);
	if (ps.bench)
		print_bench(&ps);
	free_ps(&ps);
	return (0);
}
