/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_hexdump.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 12:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/07/23 12:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_hexdump.h"

void	ft_print_line(unsigned char *line, int len, int offset)
{
	int	i;
	int	pad;

	ft_puthex((unsigned int)offset, 7);
	i = 0;
	while (i < len)
	{
		ft_putstr_fd(" ", 1);
		if (i + 1 < len)
			ft_put_group(line[i], line[i + 1]);
		else
			ft_put_group(line[i], 0);
		i += 2;
	}
	pad = (8 - (len + 1) / 2) * 5;
	while (pad > 0)
	{
		ft_putstr_fd(" ", 1);
		pad--;
	}
	ft_putstr_fd("\n", 1);
}

void	ft_feed(t_dump *d, unsigned char *buf, int n)
{
	int	i;

	i = 0;
	while (i < n)
	{
		d->line[d->len] = buf[i];
		d->len++;
		if (d->len == 16)
		{
			ft_print_line(d->line, 16, d->offset);
			d->offset = d->offset + 16;
			d->len = 0;
		}
		i++;
	}
}

void	ft_dump_fd(int fd, t_dump *d)
{
	unsigned char	buf[4096];
	int				n;

	n = read(fd, buf, 4096);
	while (n > 0)
	{
		ft_feed(d, buf, n);
		n = read(fd, buf, 4096);
	}
}

int	ft_dump_files(int argc, char **argv, t_dump *d)
{
	int	i;
	int	fd;
	int	status;

	status = 0;
	i = 1;
	while (i < argc)
	{
		fd = open(argv[i], O_RDONLY);
		if (fd < 0)
		{
			ft_error(basename(argv[0]), argv[i]);
			status = 1;
		}
		else
		{
			ft_dump_fd(fd, d);
			close(fd);
		}
		i++;
	}
	return (status);
}

int	main(int argc, char **argv)
{
	t_dump	d;
	int		status;

	d.len = 0;
	d.offset = 0;
	if (argc == 1)
	{
		ft_dump_fd(0, &d);
		status = 0;
	}
	else
		status = ft_dump_files(argc, argv, &d);
	if (d.len > 0)
		ft_print_line(d.line, d.len, d.offset);
	if (d.offset + d.len > 0)
	{
		ft_puthex((unsigned int)(d.offset + d.len), 7);
		ft_putstr_fd("\n", 1);
	}
	return (status);
}
