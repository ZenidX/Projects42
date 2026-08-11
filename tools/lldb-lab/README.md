# lldb para la piscine

Chuleta + laboratorio. Los tres binarios de aquí tienen bugs de verdad; la idea
es que los encuentres tú con el debugger, no leyendo el fuente.

```sh
cd ~/zenidx/tools/lldb-lab && make
```

---

# PARTE 0 — El examen (máquina fría, sin nada tuyo)

En el examen no hay `zentest`, ni Makefile, ni `~/.lldbinit`, ni alias. Hay un
`.c` suelto, un compilador y un reloj corriendo. Todo lo que sigue funciona en
una máquina recién instalada, sin configurar nada.

## Las tres líneas a memorizar

```sh
# 1. compilar el ejercicio + un main tuyo aparte, con simbolos
cc -Wall -Wextra -Werror -g -O0 ft_loquesea.c /tmp/m.c -o /tmp/t

# 2. segfault -> linea exacta y pila, sin entrar en modo interactivo
lldb -b -o run -o bt /tmp/t
gdb  -batch -ex run -ex bt -ex "info locals" /tmp/t     # si no hay lldb

# 3. bug de memoria (indices, malloc, strings)
cc -g -O0 -fsanitize=address ft_loquesea.c /tmp/m.c -o /tmp/t && /tmp/t
```

Con eso cubres el 90% de lo que te va a pasar en el examen. Lo demás es lujo.

## Detalles que importan en el examen

**El `main` va en otro archivo, y fuera del directorio de entrega.** El corrector
compila tu `.c` con *su* main; si dejas un `main()` en tu archivo, no compila y
suspendes el ejercicio. Escribe el tuyo en `/tmp/m.c` y enlaza los dos. Nunca
crees archivos sueltos dentro del rendu.

**Qué debugger hay.** En Mac (los iMacs del campus) está `lldb` y `gdb` no
funciona bien. En Linux está `gdb` seguro y `lldb` solo si hay clang. Aprende los
dos juegos de comandos: son cuatro palabras y no saber cuál toca te cuesta el
examen. Comprueba en 2 segundos con `which lldb gdb`.

**Equivalencias mínimas:**

| Quiero | lldb | gdb |
|---|---|---|
| arrancar | `run` | `run` |
| parar en una función | `b ft_split` | `b ft_split` |
| parar en una línea | `b fich.c:80` | `b fich.c:80` |
| siguiente línea | `n` | `n` |
| entrar en la función | `s` | `s` |
| continuar | `c` | `c` |
| pila de llamadas | `bt` | `bt` |
| **ver locales** | `fr v` | `info locals` |
| **ver argumentos** | `fr v` (salen juntos) | `info args` |
| evaluar algo | `p i + 1` | `p i + 1` |
| salir | `q` | `q` |

Lo único que cambia de verdad es `fr v` ↔ `info locals` / `info args`. El resto
es idéntico. Sin alias, sin config: estos comandos existen tal cual en cualquier
máquina.

## Presupuesto de tiempo: cuándo NO abrir el debugger

En un examen con cuenta atrás, el debugger no siempre gana:

- **Segfault** → debugger, siempre. `lldb -b -o run -o bt` te da la línea en 3
  segundos. Un `printf` a ciegas te cuesta 5 minutos.
- **Se cuelga (bucle infinito)** → debugger. `run`, esperas, `Ctrl-C`, `bt`. Te
  dice exactamente en qué bucle está atrapado.
- **Resultado incorrecto en algo corto** → `printf` suele ser más rápido. No seas
  purista: mete el printf, míralo, bórralo.
- **Resultado incorrecto y no entiendes por qué** → ahí sí, breakpoint y
  `fr v` / `info locals` para ver las locales de verdad en vez de las que crees.

**Warnings primero, siempre.** Compila con `-Wall -Wextra -Werror` desde el
minuto uno. La mitad de los segfaults del examen (puntero sin inicializar, sin
`return`, índice de tipo raro) te los canta `cc` gratis y en el acto.

**Si no hay ningún debugger instalado**: `printf` a stderr, que no se bufferiza y
por tanto sobrevive al crash — con `stdout` pierdes las últimas líneas justo
cuando más falta hacen.

```c
fprintf(stderr, "i=%d s[i]=%c\n", i, s[i]);
```

## Cómo entrenar esto

No te aprendas la tabla: hazlo con los dedos hasta que salga sin pensar. Los tres
binarios del lab valen para eso. Ponte un cronómetro y busca **bajar de 30
segundos** desde `./seg` peta hasta que sabes la línea y el valor culpable.
Repítelo unos días hasta que el `lldb -b -o run -o bt` lo teclees sin leer.

---

# PARTE 1 — El resto (con calma, en casa)

## Lo mínimo que hay que saber

Un debugger sirve para **parar el programa a mitad y mirar dentro**. Eso es todo.
Sustituye a llenar el código de `printf` y borrarlos luego.

Para que funcione, el binario tiene que llevar la info de depuración:

```sh
cc -g -O0 -o mi_prog fuente.c
```

- `-g` mete nombres de variables y números de línea en el binario.
- `-O0` apaga optimizaciones. Sin esto verás `<optimized out>` y saltos raros.

Y no, no hace falta quitar `-Wall -Wextra -Werror`: son ortogonales.

## Los 8 comandos que usarás el 95% del tiempo

| Comando | Corto | Qué hace |
|---|---|---|
| `run` | `r` | arranca el programa |
| `breakpoint set --name ft_split` | `b ft_split` | para al entrar en esa función |
| `next` | `n` | ejecuta la línea actual y para en la siguiente (sin entrar en funciones) |
| `step` | `s` | igual, pero **entra** dentro de la función que llames |
| `continue` | `c` | sigue hasta el próximo breakpoint (o hasta que pete) |
| `frame variable` | `fr v` | imprime **todas** las variables locales |
| `print i + 1` | `p i + 1` | evalúa una expresión C con las variables vivas |
| `bt` | `bt` | backtrace: la pila de llamadas hasta aquí |
| `quit` | `q` | salir |

Enter repite el último comando. Muy útil para machacar `n`.

---

## Caso 1 — segfault (`./seg`)

El reflejo correcto ante un segfault: **lanzarlo bajo lldb y pedir `bt`**. En 10
segundos sabes la línea exacta y con qué valores.

```sh
lldb ./seg
(lldb) run
```

```
Process 2911024 stopped
* thread #1, name = 'seg', stop reason = signal SIGSEGV: address not mapped to object (fault address=0x2)
    frame #0: seg`ft_strdup(s="hola mundo") at seg.c:23:12
   22  	{
-> 23  		copia[i] = s[i];
   24  		i++;
```

Ya te ha dicho tres cosas sin tocar nada:

1. **Dónde**: `seg.c:23`, dentro de `ft_strdup`.
2. **Con qué argumentos**: `s = "hola mundo"`.
3. **Qué dirección tocó**: `0x2` — una dirección minúscula, no es memoria del
   proceso. Ojo con ese dato: `0x0` o valores pequeñitos casi siempre significan
   *puntero nulo o basura*, no *me he pasado un índice*.

Ahora pregunta por el sospechoso:

```
(lldb) p copia
(lldb) bt
```

`bt` te da la cadena de llamadas (`main` → `ft_strdup`), y con
`frame select 1` subes al frame de `main` para ver sus variables. Esto es lo que
salva la vida cuando el crash es 4 funciones por debajo de donde está el fallo.

> **Pista del caso 1**: mira qué vale `copia` en la línea 23 y pregúntate quién
> le dio ese valor.

(Este en concreto ya te lo canta `cc -Wall` al compilar: *variable 'copia' is
uninitialized when used here*. Lección gratis: **lee los warnings antes de abrir
el debugger** — la mitad de los segfaults de la piscine están ahí escritos.)

## Caso 2 — resultado incorrecto sin crash (`./lab`)

Peor que un segfault: el programa termina bien y escupe basura.

```sh
./lab
dest = [#####]        # esperaba [mundo]
```

Aquí no hay señal que te pare, así que la paras tú:

```sh
lldb ./lab
(lldb) b ft_strccpy
(lldb) run
(lldb) frame variable
```

```
(char *) dest = 0x4052a0 "################################################################"
(char *) src = 0x402004 "hola mundo cruel"
(int) ini = 5
(int) fin = 10
(int) i = 0
```

Primer chequeo, siempre el mismo: **¿los argumentos son los que yo creía?** Aquí
sí. Entonces el fallo está dentro. Evalúa la condición del bucle a mano:

```
(lldb) p i
(lldb) expr fin - ini
(int) $0 = 5
```

Y ahora `n` un par de veces. Si tras el `while` acabas directamente en la línea
del `'\0'`, es que **el bucle no ha dado ni una vuelta**, y ahí está el bug: no
en el cuerpo, sino en la condición.

> **Pista del caso 2**: `i` arranca valiendo `ini`, pero la condición está escrita
> como si arrancara en 0. Decide en qué "sistema de coordenadas" trabajas —
> índices de `src` o índices de `dest` — y sé coherente en las tres líneas.

## Caso 3 — bucles largos: breakpoints con condición y watchpoints (`./words`)

Parar en la iteración 1 de 500 no sirve. Dos herramientas para eso.

**Breakpoint condicional** — para solo cuando se cumpla algo:

```
(lldb) breakpoint set -f words.c -l 34 -c 'c == 2'
(lldb) run
(lldb) p str + i
(char *) 0x402017 "cruel  "
```

`p str + i` imprimiendo *lo que queda de string* es un truco que gasta mucho:
te sitúa dentro del recorrido de un vistazo.

**Watchpoint** — no para en una línea, para **cuando una variable cambia**,
venga el cambio de donde venga:

```
(lldb) b ft_count_words
(lldb) run
(lldb) watchpoint set variable c
(lldb) c
```

```
Watchpoint 1 hit:
old value: 2
new value: 3
```

Esto es *la* herramienta para "¿quién demonios me está machacando esta
variable?". Ojo: un watchpoint sobre una local muere cuando la función retorna.

---

## Recetas sueltas

```sh
lldb -- ./runner ../repo      # pasarle argumentos al programa (el -- es clave)
lldb -p 12345                 # engancharse a un proceso que ya corre (p.ej. colgado)
```

Dentro de la sesión:

```
b ft_split.c:80          # breakpoint por archivo:línea
br list                  # listar breakpoints
br del 1                 # borrar el 1
finish                   # terminar la función actual y volver al que llamó
frame select 1           # moverse por la pila tras un bt
x/16xb ptr               # volcar 16 bytes en hexadecimal desde ptr
p *tab@3                 # imprimir 3 elementos de un array/puntero
memory read --format c --size 1 --count 20 str
```

Para un programa que **se cuelga** en vez de petar: lánzalo con `run`, deja que
se quede frito, `Ctrl-C` en lldb y `bt`. Te dice en qué bucle está atrapado.

## Un truco de arranque (en casa; en el examen esto no existe)

Crea `~/.lldbinit` con:

```
settings set target.x86-disassembly-flavor intel
command alias fv frame variable
command alias bfl breakpoint set -f
```

Y si prefieres no escribir el mismo arranque cada vez, lldb acepta comandos por
línea de órdenes (`-o`) y salir al final (`-b`):

```sh
lldb -b -o run -o bt ./seg     # "corre y dame el backtrace del crash"
```

Ese one-liner es la forma más rápida de diagnosticar un segfault. Memorízalo.

## lldb no lo ve todo

lldb te dice **dónde** revienta, no siempre **por qué**. Para memoria hay dos
compañeros que detectan el error *en el momento en que lo cometes*, no 200 líneas
después:

```sh
cc -g -O0 -fsanitize=address -o seg seg.c && ./seg   # buffer overflow, use-after-free
valgrind ./lab                                        # leaks y lecturas no inicializadas
```

Regla práctica de la piscine: si el bug huele a memoria (malloc, índices,
strings), tira primero de `-fsanitize=address`; si huele a lógica (el algoritmo
está mal), tira de lldb.
