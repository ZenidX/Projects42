/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_tail.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 12:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/07/23 12:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_TAIL_H
# define FT_TAIL_H

# include <unistd.h>
# include <fcntl.h>
# include <errno.h>
# include <string.h>
# include <libgen.h>

int		ft_strlen(char *s);
void	ft_putstr_fd(char *s, int fd);
void	ft_error(char *prog, char *file);
int		ft_atoi(char *s);
void	ft_header(char *file);
int		ft_file_size(int fd);
void	ft_print_last(int fd, int skip);
int		ft_process(char *prog, char *file, int count);
int		ft_parse(char **argv, int argc, int *count);

#endif
