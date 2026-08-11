#include <stdlib.h>
#include <stdio.h>

int	ft_strlen(char *s)
{
	int	i;

	i = 0;
	while (s[i])
		i++;
	return (i);
}

/* devuelve una copia de s... o eso cree. */
char	*ft_strdup(char *s)
{
	char	*copia;
	int		i;

	i = 0;
	while (s[i])
	{
		copia[i] = s[i];
		i++;
	}
	copia[i] = '\0';
	return (copia);
}

int	main(void)
{
	char	*p;

	p = ft_strdup("hola mundo");
	printf("copia = [%s]\n", p);
	return (0);
}
