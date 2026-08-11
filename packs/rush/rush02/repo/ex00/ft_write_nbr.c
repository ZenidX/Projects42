/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_write_nbr.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mginesti <mginesti@student.42barcelon      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/08 17:29:39 by mginesti          #+#    #+#             */
/*   Updated: 2026/08/09 21:10:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include "rush02.h"

/*
** Escribe una palabra del diccionario, separandola de la anterior con un
** espacio. Con do_write a 0 no escribe: solo comprueba que la clave existe,
** que es lo que permite recorrer el numero entero en seco antes de imprimir
** nada. Devuelve 0 si la clave falta.
*/
int	put_word(t_dict *dict, char *key, int do_write, int *first)
{
	char	*word;

	word = find_word(dict, key);
	if (!word)
		return (0);
	if (do_write)
	{
		if (!*first)
			write(1, " ", 1);
		ft_putstr(word);
	}
	*first = 0;
	return (1);
}

/*
** Un grupo de tres cifras (1 a 999). Aqui si cabe un int, porque un grupo
** nunca pasa de 999; el numero completo no cabria. Del 0 al 20 el diccionario
** tiene clave propia, asi que se busca entera en vez de descomponerla.
*/
int	write_group(int d, t_dict *dict, int do_write, int *first)
{
	char	buf[8];
	int		r;

	if (d >= 100)
	{
		if (!put_word(dict, ft_itokey(buf, d / 100), do_write, first))
			return (0);
		if (!put_word(dict, "100", do_write, first))
			return (0);
	}
	r = d % 100;
	if (r == 0)
		return (1);
	if (find_word(dict, ft_itokey(buf, r)))
		return (put_word(dict, ft_itokey(buf, r), do_write, first));
	if (!put_word(dict, ft_itokey(buf, (r / 10) * 10), do_write, first))
		return (0);
	if (r % 10 == 0)
		return (1);
	return (put_word(dict, ft_itokey(buf, r % 10), do_write, first));
}

/*
** Recorre el numero en grupos de tres, del mas significativo al menos. Los
** grupos a cero se saltan enteros, por eso 1000000 es "one million" y no
** "one million zero thousand". Devuelve 0 si falta cualquier clave.
*/
int	write_num(char *nbr, t_dict *dict, int do_write, int *first)
{
	char	buf[48];
	int		len;
	int		exp;
	int		v;

	len = ft_strlen(nbr);
	exp = ((len + 2) / 3 - 1) * 3;
	while (exp >= 0)
	{
		v = group_value(nbr, len, exp);
		if (v > 0)
		{
			if (!write_group(v, dict, do_write, first))
				return (0);
			if (exp > 0)
			{
				if (!put_word(dict, scale_key(buf, exp), do_write, first))
					return (0);
			}
		}
		exp -= 3;
	}
	return (1);
}

/*
** Quita los ceros de delante y hace dos pasadas: la primera en seco, para
** saber si el diccionario resuelve el numero entero, y solo si sobrevive la
** segunda escribiendo. Asi un Dict Error nunca sale a media frase.
*/
void	ft_write_nbr(char *nbr, t_dict *dict)
{
	int	first;

	while (nbr[0] == '0' && nbr[1])
		nbr++;
	first = 1;
	if (nbr[0] == '0')
	{
		if (!put_word(dict, "0", 1, &first))
			ft_putstr("Dict Error\n");
		else
			write(1, "\n", 1);
		return ;
	}
	if (!write_num(nbr, dict, 0, &first))
	{
		ft_putstr("Dict Error\n");
		return ;
	}
	first = 1;
	write_num(nbr, dict, 1, &first);
	write(1, "\n", 1);
}
