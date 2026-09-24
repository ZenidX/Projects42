/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_display_file.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <xalara@student.42barcelona.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/23 12:00:00 by xalara            #+#    #+#             */
/*   Updated: 2026/07/23 12:00:00 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <fcntl.h>

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

static void	ft_read_file(int fd)
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
	int	fd;

	if (argc < 2)
	{
		ft_putstr_fd("File name missing.\n", 2);
		return (1);
	}
	if (argc > 2)
	{
		ft_putstr_fd("Too many arguments.\n", 2);
		return (1);
	}
	fd = open(argv[1], O_RDONLY);
	if (fd < 0)
	{
		ft_putstr_fd("Cannot read file.\n", 2);
		return (1);
	}
	ft_read_file(fd);
	close(fd);
	return (0);
}
