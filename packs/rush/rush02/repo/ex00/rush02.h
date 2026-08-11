/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   rush02.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 03:03:44 by xalara            #+#    #+#             */
/*   Updated: 2026/08/09 19:49:51 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

typedef struct s_dict
{
	char	*nbr;
	char	*word;
}	t_dict;

int		ft_is_positive(char *str);
int		ft_strlen(char *str);
void	ft_putstr(char *str);
int		ft_strcmp(char *s1, char *s2);
int		ft_strncmp(char *s1, char *s2, unsigned int n);
char	**ft_split(char *str, char *charset);
char	*ft_strifcpy(char *src, int ini, int fin);
t_dict	*ft_read_dict(char *dict_name);
char	*ft_read_file(int fd);
int		ft_arrlen(char **arr);
void	ft_free_split(char **arr);
void	ft_free_dict(t_dict *dict);
char	*ft_strcat(char *s1, char *s2);
char	*ft_outspace(char *str);
void	ft_write_nbr(char *nbr, t_dict *dict);
char	*find_word(t_dict *dict, char *key);
char	*ft_itokey(char *buf, int n);
char	*scale_key(char *buf, int exp);
int		group_value(char *nbr, int len, int exp);
