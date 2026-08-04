# zenidx

Un espacio de trabajo para los packs de ejercicios de 42: eliges un pack una
vez y todos los paneles de Terminator se apuntan solos a él — árbol de
ficheros, norminette y tests, refrescándose mientras editas.

```
+---------------+-------------+---------------+
|               | watch tree  |               |
| zenpack       +-------------+ asistente     |
|   + shell     | norminette  |               |
|               +-------------+---------------+
|               | watch tests | shell del pack|
+---------------+-------------+---------------+
```

## Instalación

Vive dentro de [ZenidX/Projects42](https://github.com/ZenidX/Projects42), en
`tools/zen/`. Para instalarlo en otra máquina:

```sh
git clone https://github.com/ZenidX/Projects42.git ~/zenidx
cd ~/zenidx/tools/zen
./install.sh --dry-run             # mira qué haría
./install.sh
```

Si solo quieres el tooling y no los ejercicios, copia la carpeta `tools/zen/`
a donde sea y lanza el `install.sh` desde ahí: no depende de estar dentro del
repo.

Abre una terminal nueva y ya tienes `zen`, `zenlayout`, `zenpack`, `zentest`.

Es idempotente: relánzalo para actualizar. Todo lo que sobrescribe se guarda
antes con sufijo `.bak`. Si ya usas Terminator, **solo se sustituye la sección
`[layouts]`** de tu config; tu perfil, tus keybindings y tu `global_config` se
quedan como están.

### Dependencias

Imprescindibles: `terminator`, `watch` (procps), `awk`, `sed`.
Opcionales: `tree`, `norminette`, `cc`, `git` — sin ellos solo falla el panel
que los use.

```sh
sudo apt install terminator procps gawk tree
```

## Uso

```sh
zen           menú de configuración del espacio
zenlayout     abre Terminator con el layout configurado
zenpack       elige o crea un pack
zentest       lanza los tests del pack activo
zengit        git para los repos de entrega (vogsphere)
zcd repo      salta al repo del pack activo
```

El menú de `zen` agrupa lo que se toca a diario: el layout, los intervalos de
refresco de cada panel de vigilancia y el pack activo. Al aplicar, regenera la
sección `[layouts]` de Terminator y puede relanzarlo.

### Layouts

| Nombre    | Paneles | Para qué                                    |
|-----------|---------|---------------------------------------------|
| `default` | 6       | el completo, con panel de asistente          |
| `focus`   | 4       | sin asistente: pantallas pequeñas            |
| `tests`   | 1       | solo los tests, a pantalla completa          |
| `solo`    | 1       | una terminal suelta dentro del pack          |

`zenlayout -l focus` abre uno concreto sin cambiar la configuración.

## Cómo está montado

Seis scripts independientes que se comunican por **un fichero de estado**:

```
~/.local/state/zenidx/current-pack     <- lo escribe zenpack, lo leen todos
```

```
zenpack    elige el pack y lo escribe en el estado        (el único que escribe)
zenin      ejecuta un comando dentro del pack activo      (la primitiva)
zenwatch   envuelve zenin en `watch -n`                   (los paneles)
zentest    compila y lanza el runner de tests del pack
zenshell   shell interactiva dentro del pack
zenlayout  abre Terminator con el layout configurado
zengit     git para los repos de entrega, con el .git fuera del árbol
zen        menú de configuración; genera los layouts
```

`zengit` resuelve un problema concreto: el `.git` de cada repo de vogsphere no
vive dentro del pack, sino en `~/.local/share/zenidx-vogsphere/<slug>.git` con
`core.worktree` apuntando de vuelta. Así el repo de `~/zenidx` puede versionar
los ficheros de los ejercicios como ficheros normales en lugar de como un
gitlink vacío, y las entregas siguen funcionando igual. Te sitúas dentro del
pack y llamas a `zengit status`, `zengit push`, lo que sea. `zengit -l` lista
los repos gestionados.

Como cada panel resuelve el pack **en cada vuelta del watch**, cambiar de pack
con `zenpack` reapunta los seis sin reiniciar nada.

### Dónde queda cada cosa

```
~/.local/bin/zen*                       los scripts
~/.local/share/zenidx/layouts.conf.in   plantilla de layouts
~/.local/share/zenidx/test/Makefile     semilla para packs nuevos
~/.local/share/zenidx/zenidx.sh         PATH + función zcd
~/.config/zenidx/config                 ajustes (los edita `zen`)
~/.config/terminator/config             solo la sección [layouts]
~/.local/state/zenidx/current-pack      pack activo
~/zenidx/packs/                         tus ejercicios
```

## Estructura de un pack

```
~/zenidx/packs/CATEGORIA/NOMBRE/
├── repo/          el repo que entregas (ex00, ex01, ...)
├── test/          Makefile + test.c -> compila `runner`
├── subject.pdf
└── extra/
```

`zentest` equivale a `cd <pack>/test && make && ./runner ../repo`. El `test.c`
es **propio de cada pack** (sus casos prueban esos ejercicios); el instalador
solo deja el `Makefile`, que sí es común. `zenpack -n` crea un pack nuevo y le
copia el Makefile del pack más reciente, o el de la plantilla si es tu primera
instalación.

## Desinstalar

```sh
./uninstall.sh          quita scripts, plantillas y ajustes
./uninstall.sh --all    además restaura el config de terminator del .bak
```

Nunca toca `~/zenidx/packs`: tus ejercicios se quedan donde están. La línea
`# zenidx` de tu `.zshenv`/`.bashrc` hay que quitarla a mano.

## Notas

- El `PATH` va en `~/.zshenv` y no en `~/.zshrc` a propósito: los paneles de
  Terminator arrancan la shell en modo no interactivo (`zsh -c`), que no lee
  `.zshrc`. Si el `PATH` viviera allí, los paneles no encontrarían `zenwatch`.
- `watch -n N` espera N segundos **entre pasadas**, no desde el inicio de una a
  la siguiente. Como cada pasada de `zentest` recompila el runner, el periodo
  real es N + lo que tarde la compilación. Bajar de 2 a 0.5 quita espera muerta,
  pero no da refrescos a 2 Hz.
- `norminette` arranca un Python en cada pasada y es el panel más caro. Si notas
  el portátil caliente, súbelo a 5s desde `zen`.
- `zcd` es una función de shell y no un script porque un `cd` en un proceso hijo
  no afectaría a tu shell.
