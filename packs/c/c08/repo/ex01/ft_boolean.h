/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_boolean.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: xalara <zenid77@gmail.com>                 +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/09 02:37:34 by xalara            #+#    #+#             */
/*   Updated: 2026/08/10 16:00:04 by xalara           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <unistd.h>

#define TRUE 1
#define FALSE 0
#define SUCCESS 1
#define EVEN_MSG "I have an even number of arguments" 
#define ODD_MSG "I have an odd number of arguments"
#define EVEN(x) (x % 2 == 0)

typedef	int	t_bool;
