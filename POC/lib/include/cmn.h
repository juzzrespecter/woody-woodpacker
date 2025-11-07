/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmn.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alvjimen <alvjimen@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/01/30 17:50:43 by alvjimen          #+#    #+#             */
/*   Updated: 2023/04/22 16:11:38 by alvjimen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CMN_H
# define CMN_H
# define MALLOC_LIMIT 1671156
# include <stdlib.h>//malloc
# include <stdint.h>//uint
# include <limits.h>//MAX_INT
# include <stdbool.h>//bool

void		*ft_calloc(size_t count, size_t size);
void		ft_swap(void **ptr1, void **ptr2);
void		ft_free(void **ptr);
const void	*ft_ternary(const bool condition, const void *true_val,
		       const void *false_val);
bool		is_between(uint64_t lower_limit, uint64_t upper_limit,
			   uint64_t value);
bool		ft_in_array(void **arr, uint64_t arr_size, void *value);

#endif
