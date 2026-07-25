/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_tail.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 12:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/07/23 12:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_tail.h"

int	ft_file_size(int fd)
{
	char	buf[4096];
	int		total;
	int		n;

	total = 0;
	n = read(fd, buf, 4096);
	while (n > 0)
	{
		total = total + n;
		n = read(fd, buf, 4096);
	}
	return (total);
}

void	ft_print_last(int fd, int skip)
{
	char	buf[4096];
	int		n;
	int		pos;

	pos = 0;
	n = read(fd, buf, 4096);
	while (n > 0)
	{
		if (pos + n > skip)
		{
			if (pos >= skip)
				write(1, buf, n);
			else
				write(1, buf + (skip - pos), n - (skip - pos));
		}
		pos = pos + n;
		n = read(fd, buf, 4096);
	}
}

int	ft_process(char *prog, char *file, int count)
{
	int	fd;
	int	size;
	int	skip;

	fd = open(file, O_RDONLY);
	if (fd < 0)
	{
		ft_error(prog, file);
		return (1);
	}
	size = ft_file_size(fd);
	close(fd);
	skip = 0;
	if (count < size)
		skip = size - count;
	fd = open(file, O_RDONLY);
	ft_print_last(fd, skip);
	close(fd);
	return (0);
}

int	ft_parse(char **argv, int argc, int *count)
{
	int	i;

	i = 1;
	while (i < argc)
	{
		if (argv[i][0] == '-' && argv[i][1] == 'c')
		{
			if (argv[i][2])
				*count = ft_atoi(argv[i] + 2);
			else
			{
				i++;
				*count = ft_atoi(argv[i]);
			}
			return (i + 1);
		}
		i++;
	}
	return (-1);
}

int	main(int argc, char **argv)
{
	int	count;
	int	i;
	int	multi;
	int	printed;

	count = 0;
	i = ft_parse(argv, argc, &count);
	if (i < 0)
		return (1);
	multi = (argc - i > 1);
	printed = 0;
	while (i < argc)
	{
		if (multi)
		{
			if (printed)
				ft_putstr_fd("\n", 1);
			ft_header(argv[i]);
		}
		ft_process(basename(argv[0]), argv[i], count);
		printed = 1;
		i++;
	}
	return (0);
}
