/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_cat.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 12:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/07/23 12:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>

static int	ft_strlen(char *s)
{
	int	i;

	i = 0;
	while (s[i])
		i++;
	return (i);
}

static void	ft_putstr_fd(char *s, int fd)
{
	write(fd, s, ft_strlen(s));
}

static void	ft_error(char *prog, char *file)
{
	ft_putstr_fd(prog, 2);
	ft_putstr_fd(": ", 2);
	ft_putstr_fd(file, 2);
	ft_putstr_fd(": ", 2);
	ft_putstr_fd(strerror(errno), 2);
	ft_putstr_fd("\n", 2);
}

static void	ft_cat_fd(int fd)
{
	char	buf[4096];
	int		n;

	n = read(fd, buf, 4096);
	while (n > 0)
	{
		write(1, buf, n);
		n = read(fd, buf, 4096);
	}
}

int	main(int argc, char **argv)
{
	int	i;
	int	fd;
	int	status;

	status = 0;
	i = 1;
	if (argc == 1)
		ft_cat_fd(0);
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
			ft_cat_fd(fd);
			close(fd);
		}
		i++;
	}
	return (status);
}
