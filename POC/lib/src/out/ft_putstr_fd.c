/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_putstr_fd.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alvjimen <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/06/25 13:54:00 by alvjimen          #+#    #+#             */
/*   Updated: 2022/06/30 14:31:36 by alvjimen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	ft_putstr_fd(char *s, int fd)
{
	size_t	len;

	if (!s || write(fd, NULL, 0))
		return ;
	len = 0;
	while (s && s[len])
		len ++;
	if (write(fd, s, len) < (ssize_t)len)
		return ;
}
