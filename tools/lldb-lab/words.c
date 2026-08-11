#include <stdio.h>

int	ft_in_charset(char c, char *charset)
{
	int	i;

	i = 0;
	while (charset[i])
	{
		if (c == charset[i])
			return (1);
		i++;
	}
	return (0);
}

/* cuenta palabras separadas por cualquier char de charset */
int	ft_count_words(char *str, char *charset)
{
	int	c;
	int	dentro;
	int	i;

	c = 0;
	dentro = 0;
	i = 0;
	while (str[i])
	{
		if (ft_in_charset(str[i], charset))
			dentro = 0;
		else if (dentro == 0)
		{
			dentro = 1;
			c++;
		}
		i++;
	}
	return (c);
}

int	main(void)
{
	printf("%d\n", ft_count_words("  hola   mundo cruel  ", " "));
	return (0);
}
