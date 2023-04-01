/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_intlen.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aihya <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2018/10/23 05:39:43 by aihya             #+#    #+#             */
/*   Updated: 2018/10/28 15:34:17 by aihya            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

int		ft_intlen(int n)
{
	int		len;

	len = 1;
	if (n == -2147483648)
		return (11);
	if (n < 0)
	{
		len++;
		n *= (-1);
	}
	while ((n = n / 10))
		len++;
	return (len);
}