# Projects42 — xalara @ 42 Barcelona

Enunciados y soluciones de los proyectos de la cuenta **xalara** en la intra de 42.
Reconstruido el 2026-07-21 (la carpeta llevaba vacía desde enero).

## Estado en la intra

| Proyecto | Estado intra | Aquí |
|---|---|---|
| C Piscine Shell 00 | In progress | `shells/shell00/` — resuelto (ex00–ex09) |
| C Piscine Shell 01 | In progress | `shells/shell01/` — resuelto (ex01–ex08; ex00 es apuntarse al examen) |
| C Piscine C 00 | no inscrito | `c/c00/` — resuelto (ex00–ex08, ft_putchar → ft_print_combn) |
| C Piscine C 01 | no inscrito | `c/c01/` — resuelto (ex00–ex08, punteros: ft_ft → ft_sort_int_tab) |

Los módulos de C cumplen la Norma: header 42 (login xalara), tabs, sin `for`, máx. 25
líneas/función. Verificado con **norminette** (`-R CheckForbiddenSourceHeader`): 18/18 OK.
Compilados con `cc -Wall -Wextra -Werror` y testeados (19 checks, incl. INT_MIN en
ft_putnbr y las combinatorias contra generadores de referencia en Python).
Los `.c` se generan con `_build/gen_c_files.py` (el header 42 se construye ahí);
tests en `_build/test_c.sh` (corren en docker `python:3.12` en zenidx-linux).

## Estructura

- `shells/shellNN/subject.pdf` + `subject.txt` — enunciado oficial (ES) y su texto extraído.
- `shells/shellNN/exMM/` — solo los archivos a entregar de cada ejercicio.
- `shells/shell00/resources/` — material del proyecto (`a`, `sw.diff` para ex07).
- `_keys/id_ed25519_shell00` — clave privada de la pareja generada para shell00/ex03
  (la pública entregable es `shells/shell00/ex03/id_ed25519_pub`).
- `_build/` — scripts de construcción y test (se ejecutan en WSL Ubuntu).

## Notas de plataforma (Windows/NTFS)

Tres entregables no pueden existir "sueltos" en NTFS y por eso están empaquetados,
generados desde WSL con permisos y fechas POSIX reales:

- `shell00/ex01/testShell00.tar` — contiene `testShell00` (`-r--r-xr-x`, 40 bytes, Jun 1 23:42).
- `shell00/ex02/exo2.tar` — los 7 test0..test6 con sus permisos, hard link y symlink.
- `shell01/ex05/ex05.tar` — el archivo `"\?$*'MaRViN'*$?\"` (nombre ilegal en Windows);
  se regenera con `ex05/create_marvin.sh` en Linux.

Todo lo demás son ficheros de texto/scripts normales (con final de línea LF).

## Verificación

`_build/_build_and_test.sh` construye los artefactos y testea cada ejercicio en WSL
(18 comprobaciones). Última ejecución completa: 2026-07-21, todo en verde.

Para entregar en 42 habría que clonar el repo de vogsphere de cada proyecto
(la intra da la URL `git@vogsphere...` en la página del proyecto), copiar dentro las
carpetas `exNN/` y hacer push. Requiere subir antes la clave pública de `ex03` a la
intra (Settings → SSH keys).
