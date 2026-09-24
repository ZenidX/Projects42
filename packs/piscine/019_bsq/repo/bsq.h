/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bsq.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/11 01:46:47 by xalara            #+#    #+#             */
/*   Updated: 2026/08/11 07:26:07 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BSQ_H
# define BSQ_H

# include <unistd.h>
# include <stdlib.h>

/*
** obj[0] vacio, obj[1] obstaculo, obj[2] lleno (el orden de la cabecera).
** map[0] es la cabecera: las filas del mapa van de map[1] a map[height].
** sq[0] fila, sq[1] columna, sq[2] lado del mayor cuadrado encontrado.
*/
typedef struct s_map
{
	char	obj[3];
	char	**map;
	int		width;
	int		height;
	int		sq[3];
}	t_map;

int		ft_arrlen(char **arr);
void	ft_free_split(char **arr);
void	ft_free_map(t_map *map);
char	*ft_strfreecat(char *tmp_dest, char *a);
char	*ft_strifcpy(char *src, int ini, int fin);
int		ft_strlen(char *str);
void	ft_putstr(char *str);
int		ft_atoi(char *str);
int		ft_is_pos(int nbr);
char	**ft_split(char *str, char *charset);
t_map	*ft_parse_error(t_map *map);
int		ft_is_map(char c, t_map *map);
int		ft_only_digits(char *s, int n);
int		ft_check_obj(t_map *map);
int		ft_parse_header(t_map *map);
t_map	*ft_validate_map(t_map *map);
t_map	*ft_parse_map(char *map_str);
char	*ft_read_file(int fd);
t_map	*ft_map_from_fd(int fd);
t_map	*ft_read_map(char *map_name);
void	ft_run_map(t_map *map);
void	ft_validate_sq(t_map *map, int i[2], int *tmp);
void	ft_bsq_isolver(t_map *map);
void	ft_fill_sq(t_map *map);
void	ft_print_map(t_map *map);

#endif
