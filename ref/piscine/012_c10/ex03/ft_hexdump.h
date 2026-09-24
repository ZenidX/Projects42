/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_hexdump.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 12:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/07/23 12:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_HEXDUMP_H
# define FT_HEXDUMP_H

# include <unistd.h>
# include <fcntl.h>
# include <errno.h>
# include <string.h>
# include <libgen.h>

typedef struct s_dump
{
	unsigned char	line[16];
	int				len;
	int				offset;
}	t_dump;

int		ft_strlen(char *s);
void	ft_putstr_fd(char *s, int fd);
void	ft_error(char *prog, char *file);
void	ft_puthex(unsigned int value, int digits);
void	ft_put_group(unsigned char b0, unsigned char b1);
void	ft_print_line(unsigned char *line, int len, int offset);
void	ft_feed(t_dump *d, unsigned char *buf, int n);
void	ft_dump_fd(int fd, t_dump *d);
int		ft_dump_files(int argc, char **argv, t_dump *d);

#endif
