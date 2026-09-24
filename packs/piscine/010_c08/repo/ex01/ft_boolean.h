/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_boolean.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 02:37:34 by xalara            #+#    #+#             */
/*   Updated: 2026/08/12 12:19:22 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_BOOLEAN_H
# define FT_BOOLEAN_H

# include <unistd.h>

# define TRUE 1
# define FALSE 0
# define SUCCESS 0
# define EVEN_MSG "I have an even number of arguments.\n" 
# define ODD_MSG "I have an odd number of arguments.\n"
# define EVEN(x) ((x) % 2 == 0)
# define ODD(X) ((x) % 2 !=0)

typedef	int	t_bool;

#endif
