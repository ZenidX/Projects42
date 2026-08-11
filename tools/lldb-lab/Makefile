# Laboratorio de lldb: tres binarios con bugs de verdad, compilados con -g -O0.
#
#   make            construye los tres
#   make lab        [BUG LOGICO]  copia mal un trozo de string
#   make seg        [SEGFAULT]    ft_strdup sin malloc
#   make words      [ESTADO]      contador de palabras para practicar bp/watch
#
# -g mete la info de depuracion (nombres de variables, numeros de linea).
# -O0 apaga las optimizaciones: sin esto lldb te dice "variable optimized out"
# y las lineas saltan en desorden.
CFLAGS = -Wall -Wextra -g -O0

all: lab seg words

lab: main.c ft_strccpy.c
	cc $(CFLAGS) -o lab main.c ft_strccpy.c

seg: seg.c
	cc $(CFLAGS) -o seg seg.c

words: words.c
	cc $(CFLAGS) -o words words.c

clean:
	rm -f lab seg words

re: clean all

.PHONY: all clean re
