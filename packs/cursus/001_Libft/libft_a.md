# libft.a: qué es y cómo se construye

Explicado desde cero. Cada término, comando y parámetro aparece en orden, de forma que cada cosa solo use conceptos ya explicados antes.

---

## 1. Conceptos básicos

### Código fuente (`.c`)
Es el texto que escribes tú en C, como `ft_strlen.c`. El ordenador **no puede ejecutarlo directamente**: primero hay que traducirlo.

### Código máquina
Son las instrucciones en binario (ceros y unos) que entiende el procesador. Es el resultado de esa traducción.

### Compilar
Traducir código fuente a código máquina. Lo hace el **compilador**.

### `cc`
Es el nombre del **compilador de C**. Según la máquina, `cc` apunta a `clang` o a `gcc`. El subject exige usar `cc`.

### Enlazar (linkar)
Es el paso que va **después** de compilar. Cuando un programa usa varias funciones repartidas en varios archivos, el **enlazador** (*linker*) las junta en un único ejecutable y conecta cada llamada (por ejemplo, `ft_strlen(...)` en tu `main`) con el código de esa función.

Normalmente `cc` hace los dos pasos seguidos sin que lo notes:

```
main.c ──compilar──► código máquina ──enlazar──► a.out (ejecutable)
```

### Ejecutable
El archivo final que puedes lanzar, por ejemplo `./a.out`. Para que exista, el enlazador necesita encontrar una función `main`, porque es el punto donde empieza el programa.

---

## 2. Archivos objeto (`.o`)

### Qué es un `.o`
Un **archivo objeto** es el resultado de **compilar sin enlazar**. Contiene el código máquina de lo que había en **un** `.c`, pero todavía no es un ejecutable:
- No necesita `main`.
- Si llama a funciones de otros archivos, esas llamadas quedan "pendientes" hasta que el enlazador las resuelva.

### El comando

```sh
cc -Wall -Wextra -Werror -c ft_strlen.c
```

| Parte | Significado |
|---|---|
| `cc` | El compilador |
| `-Wall` | *Warnings all*: activa los avisos más comunes (variables sin usar, etc.) |
| `-Wextra` | Activa avisos adicionales que `-Wall` no incluye (parámetros sin usar, comparar con y sin signo, etc.) |
| `-Werror` | Trata **cada aviso como un error**: si hay un solo warning, no se genera nada. 42 exige estos tres flags |
| `-c` | **Solo compila, no enlaza.** El resultado es un `.o` en lugar de un ejecutable. Por eso no hace falta `main` |
| `ft_strlen.c` | El archivo fuente a compilar |

**Resultado:** aparece `ft_strlen.o` en la misma carpeta. Por defecto se llama igual que el `.c`, cambiando la extensión.

---

## 3. Librerías

### Qué es una librería
Es una colección de funciones ya compiladas, preparada para que **otros programas** las usen. Ya usas una sin darte cuenta: la **libc** (la librería estándar de C), que es donde están `write`, `malloc`, `strlen`, etc.

### Estática vs dinámica
Hay dos tipos:
- **Estática** (`.a`): cuando compilas tu programa, el código de las funciones que usas se **copia dentro** del ejecutable. Después, el ejecutable ya no necesita la librería.
- **Dinámica** (`.so` en Linux): el ejecutable solo guarda una referencia, y la función se carga al ejecutar el programa.

**`libft.a` es estática.** Por eso termina en `.a`.

### Qué es `libft.a` por dentro
Un **archivo contenedor** (*archive*, de ahí la `.a`) que guarda muchos `.o` juntos. Se parece a un `.zip`, pero sin comprimir y pensado para el enlazador:

```
ft_isalpha.c ─┐                ┌─ ft_isalpha.o ─┐
ft_memset.c  ─┼── cc -c ──────►├─ ft_memset.o  ─┼── ar ──► libft.a
ft_strlen.c  ─┘                └─ ft_strlen.o  ─┘
   (fuente)                        (objetos)              (librería)
```

### Por qué el nombre empieza por `lib`
Es una convención de Unix: toda librería se llama `lib<nombre>.a`. Esta librería se llama **`ft`**, y el archivo es `lib` + `ft` + `.a`. Esto importa luego para `-lft` (sección 5).

---

## 4. Crear la librería: `ar`

### `ar`
*Archiver*. Es la herramienta que crea y gestiona archivos `.a`. El subject obliga a usarla.

### El comando

```sh
ar rcs libft.a ft_isalpha.o ft_memset.o ft_strlen.o
```

| Parte | Significado |
|---|---|
| `ar` | El programa |
| `rcs` | Tres opciones juntas (se escriben pegadas, sin `-`): |
| → `r` | **Replace**: mete los `.o` en el archivo. Si ya había uno con el mismo nombre, lo **sustituye** por el nuevo. Así, si recompilas `ft_strlen.o`, se actualiza |
| → `c` | **Create**: si `libft.a` no existe, lo crea **sin avisar**. Sin la `c`, `ar` imprime un mensaje del tipo "creating libft.a" |
| → `s` | **Symbol index**: crea un **índice de símbolos** (ver abajo) |
| `libft.a` | El archivo de salida (el primer nombre después de las opciones) |
| `ft_*.o ...` | Los `.o` que se meten dentro |

### Símbolo e índice de símbolos
Un **símbolo** es el nombre de algo que existe en el código compilado: una función (`ft_strlen`) o una variable global.

El **índice** es una tabla al principio del `.a` que dice "el símbolo `ft_strlen` está en `ft_strlen.o`". Así el enlazador encuentra cada función sin recorrer todos los `.o`. (Hay otra herramienta, `ranlib`, que solo hace eso; la `s` la hace innecesaria.)

### Ver qué hay dentro

```sh
ar t libft.a
```

- `t`: *table*, lista los `.o` que contiene el archivo.

### `libtool`
Es otra herramienta que también puede crear librerías. **El subject la prohíbe**; hay que usar `ar`.

---

## 5. Usar la librería en otro programa

Supón que en un proyecto futuro tienes un `main.c` que llama a `ft_strlen`.

### Forma 1

```sh
cc main.c libft.a
```

- `main.c`: se compila (aquí **sin** `-c`, porque ahora sí queremos un ejecutable).
- `libft.a`: se pasa al **enlazador**, que busca dentro del índice las funciones que usa `main.c` y copia **solo esos** `.o` al ejecutable. Si `main.c` solo usa `ft_strlen`, `ft_split` no entra.

### Forma 2 (la habitual)

```sh
cc main.c -L. -lft
```

| Parte | Significado |
|---|---|
| `-L.` | Añade una carpeta donde buscar librerías. `.` es "la carpeta actual". Si estuviera en `libft/`, sería `-Llibft` |
| `-lft` | "Enlaza con la librería `ft`". El compilador añade `lib` delante y `.a` detrás, y busca `libft.a`. Por eso importa la convención del nombre |

### Por qué sigue haciendo falta `libft.h`
Son dos cosas distintas que se necesitan en momentos distintos:
- **`libft.h`** hace falta al **compilar** `main.c`. Contiene los **prototipos**, que le dicen al compilador "existe `ft_strlen`, recibe esto y devuelve aquello". Sin ellos, la llamada da error.
- **`libft.a`** hace falta al **enlazar**. Contiene el **código** de la función.

```
main.c  +  libft.h  ──compilar──►  main.o  +  libft.a  ──enlazar──►  a.out
          (prototipos)                       (código)
```

---

## 6. Lo que pide el subject

### Archivos a entregar: `Makefile`, `libft.h`, `ft_*.c`
- `ft_*.c`: el `*` es un **comodín** que significa "cualquier texto". Es decir, todos los archivos que empiezan por `ft_` y terminan en `.c`.
- **No se entregan** ni los `.o` ni `libft.a`. Son archivos **generados**, y el evaluador los crea ejecutando el Makefile.

### "`libft.a` debe crearse en la raíz de tu repositorio"
La **raíz** es la carpeta principal del repo (la de más arriba). Después de `make`, `libft.a` tiene que aparecer ahí, no en una subcarpeta.

### Makefile
Es un archivo de instrucciones para el programa **`make`**. En lugar de escribir a mano los 43 comandos `cc -c` y el `ar`, escribes las reglas una vez y luego solo ejecutas `make`.

### Regla
Cada bloque del Makefile. Tiene tres partes:

```make
objetivo: dependencias
	receta
```

- **Objetivo** (*target*): lo que se quiere conseguir, normalmente un archivo (`libft.a`) o un nombre de acción (`clean`).
- **Dependencias** (*prerequisites*): lo que tiene que existir antes. Si alguna cambia, hay que rehacer el objetivo.
- **Receta** (*recipe*): los comandos que se ejecutan. **Tienen que empezar con un tabulador**, no con espacios. Si no, `make` da error.

Se ejecuta con `make <objetivo>`. Solo `make` ejecuta la primera regla del archivo.

### Variable (`NAME`) y `$(NAME)`
Un Makefile puede tener variables:

```make
NAME = libft.a
```

Con **`$(NAME)`** se usa su valor. Donde pongas `$(NAME)`, `make` lo sustituye por `libft.a`.

### Las reglas obligatorias

| Regla | Qué debe hacer |
|---|---|
| **`$(NAME)`** | La regla que construye `libft.a`: depende de todos los `.o` y ejecuta `ar rcs` |
| **`all`** | La regla por defecto (va primera). Simplemente depende de `$(NAME)`: `make` = `make all` = crear la librería |
| **`clean`** | Borra los `.o` (con `rm -f`). Deja `libft.a` |
| **`fclean`** | *Full clean*: hace `clean` y **además** borra `libft.a`. Deja el repo como si nunca se hubiera compilado |
| **`re`** | *Rebuild*: `fclean` y después `all`. Recompila todo desde cero |

Sobre **`rm -f`**: `rm` borra archivos, y `-f` (*force*) hace que no se queje si el archivo no existe. Así `make clean` no falla aunque ya estuviera limpio.

### Relink (y por qué no debe pasar)
**Relink** significa volver a generar el resultado final cuando no hacía falta. El subject lo prohíbe.

`make` decide si rehacer algo comparando **fechas de modificación**: si `libft.a` es más reciente que todos sus `.o`, y cada `.o` es más reciente que su `.c`, no hay nada que hacer. El evaluador hará:

```sh
make
make      # esta segunda vez NO debe compilar ni ejecutar ar
```

Lo correcto la segunda vez es un mensaje como `make: Nothing to be done for 'all'.`. Si el Makefile vuelve a lanzar `cc` o `ar`, es relink y cuenta como error.

### `.PHONY`
`all`, `clean`, `fclean` y `re` **no son archivos**, son nombres de acciones. Si por casualidad existiera un archivo llamado `clean` en la carpeta, `make clean` diría "ya está hecho" y no borraría nada. Para evitarlo se declaran como "falsos":

```make
.PHONY: all clean fclean re
```

---

## Resumen del flujo completo

```
tú escribes          make ejecuta                          resultado
─────────────        ───────────────────────────────────   ─────────
ft_*.c + libft.h ──► cc -Wall -Wextra -Werror -c ft_X.c ──► ft_X.o (uno por .c)
                     ar rcs libft.a ft_*.o ──────────────► libft.a (en la raíz)
```
