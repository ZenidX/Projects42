/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/06 12:24:17 by xalara            #+#    #+#             */
/*   Updated: 2026/10/10 17:33:28 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"



char	*gnl_split(char *buf, char *p, int fd)
{
	char	*r;
	char	*q;
	
	buf 

	return (r);
}

char	*gnl_read(char *buf, char *p, char *r, int fd)
{
	ssize_t bytes_read;

	r = gnl_strcat(r, buf);
	bytes_read = 1;
	while (!gnl_strchr(buf, '\n') && bytes_read > 0)
	{
		bytes_read = read(fd, buf, BUFFER_SIZE);
		if (bytes_read < 0)
		else (bytes_read)
		{
			buf[bytes_read] = '\0';
			r = gnl_strcat(r, buf);
		}
		else
			
		i++;
	}
	write(1, r, gnl_strlen(r));
	return (r);
}

char	*get_next_line(int fd)
{
	static char	*buf = NULL;
	char		*r;

	if (fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	if (!buf)
	{
		buf = malloc(sizeof(char) * (BUFFER_SIZE + 1));
		buf[0] = '\0';
		buf[BUFFER_SIZE] = '\0';
		p = NULL;
	}
	else
		p = gnl_strchr(buf, '\n');
	if (!p)
		r = gnl_read(buf, fd);
	else
		r = gnl_split(buf, fd);
	return (r);
}
