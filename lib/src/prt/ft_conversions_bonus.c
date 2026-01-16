/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_conversions_bonus.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alvjimen <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/20 18:32:45 by alvjimen          #+#    #+#             */
/*   Updated: 2023/01/30 17:11:49 by alvjimen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */
#include "prt.h"

static int	ft_conversion_char_long(char **str, t_flags *flags, va_list args,
	ssize_t *count)
{
	if (!ft_strncmp(*str, "ld", 2) || ft_strncmp(*str, "li", 2))
		*count += ft_putdigit(va_arg(args, long int), flags);
	else if (!ft_strncmp(*str, "lu", 2))
		*count += ft_putuint(va_arg(args, unsigned long int), DEC, flags);
	else if (!ft_strncmp(*str, "lx", 2))
		*count += ft_puthex_pf(va_arg(args, unsigned long int), flags, HEX_L);
	else if (!ft_strncmp(*str, "lX", 2))
		*count += ft_puthex_pf(va_arg(args, unsigned long int), flags, HEX_L);
	else
		return (0);
    //*str[0]++; // This should be the solution i need to check further.
	return (1);
}

static void	ft_conversion_char(char **str, t_flags *flags, va_list args,
	ssize_t *count)
{
	flags->str = ft_memjoin_free_pf((void **)&flags->str, (void *)*str,
			ft_strlen_null(flags->str), 1);
	if (*str[0] == '%')
		*count += write(1, str[0], 1);
	else if (*str[0] == 'c')
		*count += ft_putchar_pf((char) va_arg(args, int), flags);
	else if (*str[0] == 's')
		*count += ft_putstr_pf(va_arg(args, void *), flags);
	else if (*str[0] == 'p')
		*count += ft_putpointer_pf(va_arg(args, size_t), flags);
	else if (*str[0] == 'd' || *str[0] == 'i')
		*count += ft_putdigit(va_arg(args, int), flags);
	else if (*str[0] == 'u')
		*count += ft_putuint(va_arg(args, unsigned int), DEC, flags);
	else if (*str[0] == 'x')
		*count += ft_puthex_pf(va_arg(args, unsigned int), flags, HEX_L);
	else if (*str[0] == 'X')
		*count += ft_puthex_pf(va_arg(args, unsigned int), flags, HEX_U);
	else if (!ft_conversion_char_long(str, flags, args, count))
		*count += ft_printf("%s", flags->str);
}

void	ft_conversion(char **str, va_list args, ssize_t *count)
{
	t_flags	*flags;

	flags = ft_calloc(1, sizeof(t_flags));
	if (!flags)
		return ;
	flags->str = ft_memjoin_free_pf((void **)&flags->str, (void *)*str,
			0, 1);
	str[0]++;
	ft_handle_flags(flags, str);
	ft_conversion_char(str, flags, args, count);
	ft_free_struct((void *)flags, ft_free_t_flags);
}
