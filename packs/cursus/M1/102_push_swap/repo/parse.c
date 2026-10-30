/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/25 01:38:51 by xalara            #+#    #+#             */
/*   Updated: 2026/10/25 01:38:51 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "push_swap.h"

/*
** Lectura de argumentos. Se lee de abajo arriba:
**   parse_args -> add_words -> add_number -> to_int
**              -> parse_flag (si el argumento empieza por "--")
** Todas devuelven 1 si va bien y 0 si hay error; el 0 sube hasta main, que
** llama a error_exit.
*/

/*
** Reconoce un flag. Las comparaciones usan longitud + 1 para meter el '\0'
** en la comparacion: asi "--simplex" no cuela como "--simple".
** --bench se puede repetir sin problema; la estrategia solo se puede fijar
** una vez (strategy_set), un segundo flag de estrategia es Error.
** Cualquier otro "--algo" tambien es Error.
*/
static int	parse_flag(t_ps *ps, char *s)
{
	t_strategy	st;

	if (ft_strncmp(s, "--bench", 8) == 0)
	{
		ps->bench = 1;
		return (1);
	}
	if (ft_strncmp(s, "--simple", 9) == 0)
		st = SIMPLE;
	else if (ft_strncmp(s, "--medium", 9) == 0)
		st = MEDIUM;
	else if (ft_strncmp(s, "--complex", 10) == 0)
		st = COMPLEX;
	else if (ft_strncmp(s, "--adaptive", 11) == 0)
		st = ADAPTIVE;
	else
		return (0);
	if (ps->strategy_set)
		return (0);
	ps->strategy = st;
	ps->strategy_set = 1;
	return (1);
}

/*
** Un atoi estricto: signo opcional y despues solo digitos, al menos uno
** ("-" o "+" solos son Error). n es long para poder comprobar el overflow
** en cada digito, antes de que se salga del int: con 2147483648 la
** comprobacion falla en el ultimo digito y se devuelve Error.
** El resultado va a *out; el return dice si era valido.
*/
static int	to_int(char *s, int *out)
{
	long	n;
	int		sign;

	sign = 1;
	if (*s == '-')
		sign = -1;
	if (*s == '+' || *s == '-')
		s++;
	if (!*s)
		return (0);
	n = 0;
	while (*s)
	{
		if (!ft_isdigit(*s))
			return (0);
		n = n * 10 + (*s++ - '0');
		if (n * sign > INT_MAX || n * sign < INT_MIN)
			return (0);
	}
	*out = (int)(n * sign);
	return (1);
}

/*
** Convierte una palabra y la mete al final de a (asi el primer numero de
** la linea de comandos queda en la cima). Se reserva el int primero y se
** libera si la palabra no es un numero o falla ft_lstnew.
** Los duplicados no se miran aqui sino en normalize().
*/
static int	add_number(t_ps *ps, char *word)
{
	int		*value;
	t_list	*node;

	value = malloc(sizeof(int));
	if (!value)
		return (0);
	node = NULL;
	if (to_int(word, value))
		node = ft_lstnew(value);
	if (!node)
	{
		free(value);
		return (0);
	}
	ft_lstadd_back(&ps->a, node);
	ps->size++;
	return (1);
}

/*
** Un argumento puede llevar varios numeros: "3 2 1" vale lo mismo que
** 3 2 1. Se parte por espacios y se anade cada palabra.
** Un argumento sin palabras ("" o "   ") da words[0] == NULL -> Error.
** Se para en el primer numero malo (ok a 0) y se libera el split siempre.
*/
static int	add_words(t_ps *ps, char *arg)
{
	char	**words;
	int		ok;
	int		i;

	words = ft_split(arg, ' ');
	if (!words)
		return (0);
	ok = (words[0] != NULL);
	i = 0;
	while (ok && words[i])
		ok = add_number(ps, words[i++]);
	free_split(words);
	return (ok);
}

/*
** Recorre los argumentos (sin argv[0]): los que empiezan por "--" son
** flags y el resto numeros, en cualquier orden. Si todo fue bien y hay
** numeros, normalize() los pasa a rangos y de paso detecta duplicados.
*/
int	parse_args(t_ps *ps, int n, char **args)
{
	int	i;
	int	ok;

	i = 0;
	ok = 1;
	while (ok && i < n)
	{
		if (ft_strncmp(args[i], "--", 2) == 0)
			ok = parse_flag(ps, args[i]);
		else
			ok = add_words(ps, args[i]);
		i++;
	}
	if (ok && ps->a)
		ok = normalize(ps);
	return (ok);
}
