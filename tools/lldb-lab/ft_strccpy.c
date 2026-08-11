#include <stdlib.h>

/* copia src[ini..fin) en dest. Tiene un bug a proposito. */
void	ft_strccpy(char *dest, char *src, int ini, int fin)
{
	int	i;

	i = ini;
	while (i < fin - ini)
	{
		dest[i] = src[ini + i];
		i++;
	}
	dest[i] = '\0';
}
