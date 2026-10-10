/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/06 12:24:52 by xalara            #+#    #+#             */
/*   Updated: 2026/10/06 21:37:02 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GET_NEXT_LINE_H
# define GET_NEXT_LINE_H

# include <unistd.h>
# include <stdlib.h>

# ifndef BUFFER_SIZE
#  define BUFFER_SIZE 42
# endif

char	*get_next_line(int fd);

size_t	gnl_strlen(char *r);
void	gnl_strcat(char *r, char *p);
void	gnl_strjoin(char *r, char **p);
void	gnl_strncpy(char *s, char *buf, ssize_t bytes_read);
char	*gnl_strchr(const char *s, int c);
char	*gnl_substr(const char *s, size_t start, size_t len);

#endif
