# libft — la maestra

Mi libft de trabajo: la que entregué en `packs/cursus/M0/001_Libft` más las funciones
que voy añadiendo en los proyectos siguientes. Es la que se copia a cada proyecto que
lleva libft (`repo/libft/`), así que **aquí no hay nada que no compile con
`-Wall -Wextra -Werror` y pase norminette**.

No se entrega nunca desde aquí: vogsphere necesita los ficheros dentro del `repo/`
del proyecto, y para eso está `zenlibft`.

```sh
zenlibft          # maestra -> repo/libft del pack activo (enseña qué cambia y pregunta)
zenlibft -b       # repo/libft del pack -> maestra (lo nuevo que hayas hecho allí)
zenlibft -l       # qué packs llevan libft y si van al día
```

`zenlibft` solo copia un fichero si en el destino no se pierde nada. Si los dos lados
han cambiado, enseña el diff y no lo toca (con `-f` lo fuerza).

## Qué hay además de Libft

Las 43 funciones del subject de Libft están tal cual se entregaron (las `ft_lst*`
como funciones normales, sin `_bonus`). Lo que se ha ido sumando:

| Función | Fichero | De dónde sale | Para qué |
|---|---|---|---|
| `ft_itoa_base` | `ft_itoa_base.c` | piscine c07 ex04, sin el signo | `unsigned long` a cadena en cualquier base; `%x %X %p %u` de ft_printf |

```c
char	*ft_itoa_base(unsigned long n, char *base, int b, int d);
```

`base` son los símbolos (`"0123456789abcdef"`), `b` cuántos hay (16) y `d` la
profundidad de la recursión: se llama siempre con `0`. Reserva con malloc (hay que
hacer `free`) y devuelve NULL si falla. No escribe el `-`: los negativos se tratan
fuera, pasándole `-(unsigned long)n`. Con `b < 2` no termina.

Al añadir una función: su `ft_X.c`, su prototipo en `libft.h`, su entrada en `SRCS`
del `Makefile` y una fila en esta tabla.
