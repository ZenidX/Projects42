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

# ifndef MAX_FD
#  define MAX_FD 1024
# endif

# ifndef CHAR_SEP
#  define CHAR_SEP '\n'
# endif

char	*get_next_line(int fd);

size_t	gnl_strlen(char *r);
char	*gnl_strchr(char *r, char c);
char	*gnl_strdup(char *s, size_t l);
void	gnl_strncpy(char *dst, char *q, size_t n);
char	*gnl_strncat(char *dst, char *src, size_t n);

#endif
