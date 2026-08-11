#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void	ft_strccpy(char *dest, char *src, int ini, int fin);

int	main(void)
{
	char	*str;
	char	*dest;

	str = "hola mundo cruel";
	/* quiero copiar la palabra "mundo": va de str[5] a str[10] */
	dest = malloc(64);
	memset(dest, '#', 64);
	ft_strccpy(dest, str, 5, 10);
	printf("dest = [%s]\n", dest);
	return (0);
}
