#include "libft.h"

int ft_isnumber(const char *str)
{
	int	len;
	int	i;

	if (str == NULL)
		return (0);
	if (str[0] == '-' || str[0] == '+')
		str++;
	len = ft_strlen(str);
	if (len == 0)
		return (0);
	i = 0;
	while (i < len)
	{
		if (!ft_isdigit(str[i]))
			return (0);
		i++;
	}
	return (1);
}
