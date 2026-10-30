# zenidx — xalara @ 42 Barcelona

Enunciados, soluciones y herramientas de los proyectos de la cuenta **xalara** en la
intra de 42, organizados por **etapa** (`piscine`, `cursus`).

## Estructura

```
packs/            los ejercicios: un directorio por proyecto
  piscine/        000_shell00 001_shell01 002_c00 … 015_c13 016_rush00 … 019_bsq
  cursus/         M0/ M1/ … un directorio por milestone (+ .milestones)
lib/libft/        tu libft maestra: Libft + lo que vas añadiendo (`zenlibft` la reparte)
ref/              material de consulta (ref/piscine/NNN_…, 42cursus.md, maestro42.md)
web/              fuente de 42.zenidx.com (portada + web/piscine/NNN_*.html)
tools/zen/        el espacio de trabajo: zen, zenpack, zentest, zengit… (ver su README)
tools/lldb-lab/   laboratorio para practicar con lldb
_build/           scripts de la época Windows/WSL (rutas `/mnt/e/…`, legado)
```

### Numeración y milestones

Dentro de cada etapa los proyectos van numerados `NNN_`, de `000` a `999`, **en el
orden en que se hacen** — que no es el alfabético, y es el que interesa ver en el menú
de `zenpack`. En piscine ese orden es shells → C → rushes → BSQ; en cursus empieza en
`000_C-piscine-reloaded`, el primer proyecto de la etapa.

El 42cursus va por **milestones** (los *ranks*, las quests `common-core-rank-00` … `06`
de la intra), y cada uno tiene su directorio `MN/` dentro de la etapa. La numeración
sigue la centena del milestone, para que el número no se repita en toda la etapa:

```
packs/cursus/M0/000_C-piscine-reloaded   rank 00 · arranque
packs/cursus/M0/001_Libft
packs/cursus/M1/100_ft_printf            rank 01 · fundamentos
packs/cursus/M1/101_get_next_line
packs/cursus/M1/102_push_swap
```

Los títulos de cada milestone están en `packs/cursus/.milestones`, y el reparto de
proyectos por rank que da el holy graph en [`ref/42cursus.md`](ref/42cursus.md).
`zenpack` pregunta primero por el milestone y luego por el proyecto, para que la lista
no crezca a cincuenta líneas. Una etapa sin directorios `MN/` (piscine) se lista entera.

El número es solo una convención de nombre: `zenpack` la entiende (`zenpack c03`,
`zenpack 005` y `zenpack 005_c03` llevan al mismo sitio) pero no la impone.

### Un pack

```
packs/ETAPA/NNN_nombre/          (o packs/ETAPA/MN/NNN_nombre/ con milestones)
├── repo/          lo que se entrega (ex00, ex01, …)
├── test/          Makefile + test.c -> compila `runner`   (`zentest`)
├── subject.pdf    enunciado oficial (algunos con subject.txt extraído)
├── extra/         material del proyecto
└── .zennorm       flags extra de norminette, si el subject obliga a saltarse alguna
```

No todos los packs tienen las cinco cosas: los de C de la segunda mitad están aún sin
resolver, y `016_rush00` entrega desde la raíz del pack en vez de desde `repo/`.

## Trabajar

```sh
zenhelp                 qué se puede hacer aquí (y `zenhelp <comando>` para el detalle)
zenpack                 elige el pack (o créalo: propone el siguiente número libre)
zenlayout               abre el espacio de Terminator apuntado al pack activo
zentest                 compila y lanza el runner del pack activo
zenlibft                copia tu libft maestra al repo/libft del pack (`-b` la trae de vuelta)
zengit push             entrega a vogsphere
```

Todo esto lo instala `tools/zen/install.sh`; el detalle está en
[`tools/zen/README.md`](tools/zen/README.md).

## Entregas

Cada proyecto se entrega a su repo de **vogsphere** (la intra da la URL `git@vogsphere…`
en la página del proyecto). El `.git` de esos repos no vive dentro del pack, sino en
`~/.local/share/zenidx-vogsphere/<etapa>-<NNN_nombre>.git` con `core.worktree` apuntando
de vuelta: así este repo versiona los ejercicios como ficheros normales y no como un
gitlink vacío. Lo gestiona `zengit`.

El slug sale de la ruta sin el `MN/` (`cursus/M0/001_Libft` → `cursus-001_Libft`), así que
**renombrar un pack obliga a renombrar también su git-dir**, y moverlo de sitio —también
de un milestone a otro— a reapuntar su `core.worktree`.

## Norma

Los módulos de C cumplen la Norma: header 42 (login xalara), tabs, sin `for`, máximo 25
líneas por función; compilados con `cc -Wall -Wextra -Werror`. El panel de norminette
del layout los vigila en cada guardado. `010_c08` lleva un `.zennorm` con
`-R CheckDefine` porque su propio subject obliga a escribir macros con parámetros y un
ternario.

## Notas de plataforma

El repo nació en Windows/NTFS y dos entregables de `000_shell00` siguen empaquetados,
generados desde WSL con permisos y fechas POSIX reales:

- `repo/ex01/testShell00.tar` — contiene `testShell00` (`-r--r-xr-x`, 40 bytes, Jun 1 23:42).
- `repo/ex02/exo2.tar` — los 7 `test0`…`test6` con sus permisos, hard link y symlink.

El resto son ficheros de texto y scripts normales, con final de línea LF.
