/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/06 12:24:17 by xalara            #+#    #+#             */
/*   Updated: 2026/10/17 03:37:57 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

char	*gnl_split(char *buf, char *p, char *r)
{
	char	*q;

	if (!r)
	{
		r = gnl_strdup("", 0);
		if (!r)
			return (NULL);
	}
	q = gnl_strdup(p + 1, gnl_strlen(p));
	if (!q)
	{
		free(r);
		return (NULL);
	}
	r = gnl_strncat(r, buf, gnl_strlen(r) + (p - buf + 1));
	gnl_strncpy(buf, q, BUFFER_SIZE + 1);
	free(q);
	return (r);
}

char	*gnl_read_loop(char *buf, int fd, char *r, ssize_t bytes_read)
{
	char	*p;

	while (r && bytes_read > 0)
	{
		buf[bytes_read] = '\0';
		p = gnl_strchr(buf, CHAR_SEP);
		if (p)
		{
			r = gnl_split(buf, p, r);
			break ;
		}
		else
		{
			r = gnl_strncat(r, buf, gnl_strlen(r) + bytes_read);
			buf[0] = '\0';
			bytes_read = read(fd, buf, BUFFER_SIZE);
		}
	}
	if (bytes_read < 0)
	{
		free(r);
		return (NULL);
	}
	return (r);
}

char	*gnl_read(char *buf, int fd)
{
	char	*r;

	r = gnl_strdup(buf, BUFFER_SIZE + 1);
	if (!r)
		return (NULL);
	buf[0] = '\0';
	r = gnl_read_loop(buf, fd, r, read(fd, buf, BUFFER_SIZE));
	return (r);
}

void	*gnl_set_buffer(char **buf, int fd)
{
	if (!buf[fd])
	{
		buf[fd] = (char *)malloc(sizeof(char) * (BUFFER_SIZE + 1));
		if (!buf[fd])
			return (NULL);
		buf[fd][0] = '\0';
		buf[fd][BUFFER_SIZE] = '\0';
	}
	return (buf[fd]);
}

char	*get_next_line(int fd)
{
	static char	*buf[MAX_FD];
	char		*p;
	char		*r;

	if (fd < 0 || fd >= MAX_FD || BUFFER_SIZE <= 0 || !gnl_set_buffer(buf, fd))
		return (NULL);
	r = NULL;
	p = gnl_strchr(buf[fd], CHAR_SEP);
	if (!p)
		r = gnl_read(buf[fd], fd);
	else
		r = gnl_split(buf[fd], p, r);
	if (!r || r[0] == '\0')
	{
		free(r);
		free(buf[fd]);
		buf[fd] = NULL;
		return (NULL);
	}
	return (r);
}
