/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_display_file.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/24 19:56:50 by xalara            #+#    #+#             */
/*   Updated: 2026/09/24 20:17:34 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>
#include <fcntl.h>

void	ft_putstr_fd(char *s, int fd)
{
	int	i;

	i = 0;
	while (s[i])
		i++;
	write(fd, s, i);
	write(fd, "\n", 1);
}

void	ft_cat_fd(int fd)
{
	char	buf[4096];
	int		n;

	n = read(fd, buf, sizeof(buf));
	while (n > 0)
	{
		write(1, buf, n);
		n = read(fd, buf, sizeof(buf));
	}
}

int	main(int argc, char **argv)
{
	int	fd;

	if (argc < 2)
	{
		ft_putstr_fd("File name missing.", 2);
		return (0);
	}
	if (argc > 2)
	{
		ft_putstr_fd("Too many arguments.", 2);
		return (0);
	}
	fd = open(argv[1], O_RDONLY);
	if (fd < 0)
	{
		ft_putstr_fd("Cannot read file.", 2);
		return (1);
	}
	ft_cat_fd(fd);
	close(fd);
	return (0);
}
