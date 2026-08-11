# Maestro42 — manual de uso

Emulador local del examen de 42. El motor es `42_EXAM` de JCluzet; Maestro42 le añade
los subjects de piscine. **No es oficial ni lo hace 42.**

Instalado en `~/maestro42`. Verificado en esta máquina: `gcc`, `g++`, `clang`, `clang++`,
`make` y `readline` presentes, el motor compila limpio y el corrector se ha probado con un
caso correcto y uno incorrecto.

---

## 1. Qué cubre de verdad

| Parte | Contenido | Alcance |
|---|---|---|
| **Piscine PART** | EXAM WEEK 01 — 22 ejercicios, niveles 0–7 | 4 h de límite |
| | EXAM WEEK 02 — 26 ejercicios, niveles 0–7 | 4 h de límite |
| **Student PART** | Exam Rank 02–06 | para más adelante |

En disco hay además `exam_03` (15 ejercicios) y `exam_04` (vacío), pero **el menú no los
ofrece**: `piscine_menu()` sólo acepta las opciones 1 y 2. Son contenido muerto.

El nivel máximo de piscine es 8, es decir niveles `0` a `7`. Completar el 7 termina el examen.

---

## 2. Arrancar

```bash
cd ~/maestro42
make
```

La primera vez pide aceptar las condiciones: escribe **`agree`** (nada más sirve, cualquier
otra cosa cierra el programa).

Dos cosas que conviene saber del arranque:

- **Si `make` parece no hacer nada, ejecútalo otra vez.** `launch.sh` empieza comprobando si
  quedó un `.system/a.out` de antes; si lo encuentra, lo borra y sale sin lanzar nada.
- **Hace `git pull` en cada arranque** si detecta internet. Estás ejecutando lo que haya
  subido upstream ese día. Si prefieres congelar la versión: `git remote remove origin`.

---

## 3. El flujo de menús

```
make
 └─ agree                      (sólo la primera vez)
    └─ 1                       Piscine PART
       └─ 1 ó 2                EXAM WEEK 01 / 02
          └─ y                 confirmar registro
             └─ Enter          pantalla de explicación
                └─ Enter       arranca el cronómetro ⏱
```

Desde el menú de piscine, `0` vuelve atrás. La opción `3` del menú principal son los ajustes.

**Trabaja en una segunda terminal.** El examshell se queda ocupando esta ventana; escribir el
código en otra es parte del planteamiento.

---

## 4. Dónde va cada cosa

Al empezar cada ejercicio se regeneran estas carpetas en la raíz de `~/maestro42`:

| Ruta | Qué es |
|---|---|
| `subjects/subject.en.txt` | el enunciado del ejercicio actual |
| `rendu/<assignment>/<fichero>.c` | **aquí escribes tú** |
| `traces/<nivel>-<intento>_<nombre>.trace` | el diff de cada fallo |
| `success/` | copia de todo lo que has aprobado |
| `.system/grading/` | solución de referencia + tester |

La subcarpeta dentro de `rendu/` **debe llamarse exactamente como el `Assignment name`** del
enunciado, y el fichero como el `Expected files`. Si el nombre no coincide, el tester no
encuentra nada y cuenta como fallo.

Ejemplo real:

```
subjects/subject.en.txt
    Assignment name  : ft_print_numbers
    Expected files   : ft_print_numbers.c

→ rendu/ft_print_numbers/ft_print_numbers.c
```

> ⚠️ `.system/grading/` contiene la **solución de referencia** del ejercicio que tienes
> delante. Es spoiler directo. Si quieres que el entrenamiento valga, no la abras.

---

## 5. Comandos del prompt `examshell>`

| Comando | Qué hace |
|---|---|
| `grademe` | corrige lo que haya en `rendu/` (pide confirmar con `y`) |
| `status` | nivel, tiempo restante, ejercicio actual |
| `help` | ayuda del propio programa |
| `settings` | menú de ajustes |
| `finish` / `exit` / `quit` | salir — **confirma escribiendo `yes`** |
| `sponsor`, `repo_git` | abren páginas del autor en el navegador |

Comandos "cheat", **desactivados por defecto** (se activan en `settings` → opción `2`):

| Comando | Qué hace |
|---|---|
| `new_ex` | cambia el ejercicio por otro del mismo nivel |
| `remove_grade_time` | elimina la espera entre correcciones |
| `force_success` | aprueba el ejercicio a la fuerza — sólo VIP |
| `gradenow` | corrige al instante, sin espera simulada — sólo VIP |

Cada uso incrementa un contador y al terminar el examen te lo echa en cara. `Ctrl+C` te
desconecta y pierdes la sesión.

---

## 6. Cómo corrige realmente

El tester de cada ejercicio hace esto:

1. Compila la solución de referencia con `gcc -o source`.
2. Compila tu fichero de `rendu/` con `gcc -o final`.
3. Ejecuta ambos con los mismos argumentos, pasa la salida por `cat -e`.
4. `diff` de las dos salidas. **Idénticas byte a byte → `passed`.**
5. Si tu binario sigue vivo a los 20 s → `TIMEOUT` (así detecta bucles infinitos).

Lo que **no** hace, y conviene tener claro:

- **No pasa norminette.**
- **No compila con `-Wall -Wextra -Werror`** — usa `gcc -o` a secas. Aquí un warning no te
  suspende. No uses este tester para validar estilo ni avisos del compilador.

Un fallo se guarda en `traces/` con este formato:

```
----------------8<-------------[ START TEST
        💻 TEST
./a.out         🔎 YOUR OUTPUT:
0123456789$
        🗝 EXPECTED OUTPUT:
0123456789----------------8<------------- END TEST ]
```

El `$` es de `cat -e`: marca final de línea. En ese ejemplo sobraba un `\n`.

---

## 7. La espera entre intentos

Reintentar **el mismo ejercicio** cuesta cada vez más tiempo, con una progresión tipo
Fibonacci:

| Intento | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|---|---|---|---|---|---|---|---|
| Espera | 30 s | 2 min 30 s | 3 min | 5 min 30 s | 8 min 30 s | 14 min | 22 min 30 s |

Encima, `grademe` simula entre 1 y 5 pausas aleatorias de hasta ~7 s cada una para imitar la
cola del servidor real. Es deliberado: te obliga a testear bien antes de pulsar.
`remove_grade_time` lo anula, pero cuenta como cheat.

---

## 8. Sesiones y reanudar

Al salir se guarda `.system/exam_token/current_token.txt`. En el siguiente `make`, si el
tiempo **no** ha expirado, aparece un menú con `1 RESTORE EXAM` / `2 ERASE EXAM`.

El repo viene con un token ya commiteado (una sesión de Exam Rank 02 de febrero de 2025).
Está caducado, así que se ignora y arrancas en el menú normal.

Targets del Makefile:

```bash
make          # lanzar
make grade    # limpiar y lanzar en modo corrección
make re       # limpiar y relanzar
make clean    # borrar el binario del motor
```

---

## 9. Privacidad

`.system/data_sender.sh` envía por `curl` a `https://user.grademe.fr/exam.php`:

- tu `$LOGNAME` y el `uname` de la máquina, con fecha y hora
- qué examen eliges
- nombre del ejercicio, nivel, y si apruebas o fallas
- si usas comandos cheat

Además, al arrancar descarga `user.grademe.fr/vip_list` para comprobar si eres VIP.

Para anonimizar: `settings` → opción `3`, que sustituye tu login por un id aleatorio
persistente. Para cortarlo del todo:

```bash
echo '#!/bin/bash' > ~/maestro42/.system/data_sender.sh
```

Sigue funcionando igual; los envíos quedan en nada.

---

## 10. Rutina recomendada

1. Lanza el examen y **no toques `.system/`**.
2. Lee `subjects/subject.en.txt` entero, incluidas las *Allowed functions*.
3. Escribe en `rendu/<assignment>/`, y **prueba con tu propio `main` antes de `grademe`** —
   la espera crece rápido.
4. Compara tu salida con `| cat -e` a mano; casi todos los fallos son un `\n` de más o de
   menos.
5. Cuando falles, lee el `.trace` antes de tocar el código.
