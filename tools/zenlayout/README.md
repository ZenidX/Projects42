# zenlayout

Entorno de trabajo para los ejercicios de 42: un layout de Terminator de 6 paneles
que se reapunta entero al *pack de ejercicios* que elijas, sin reiniciar nada.

```
+---------------+-------------+---------------+
| zenpack       | watch tree  |               |
|   -> shell en +-------------+ claude        |
|      <pack>/  | norminette  |               |
|      repo     +-------------+---------------+
|               | watch tests | shell del pack|
+---------------+-------------+---------------+
```

Un **pack** es un directorio `CATEGORIA/NOMBRE` bajo `~/zenidx` con la forma
`repo/ test/ subject.pdf extra/` — por ejemplo `c/c03`, `rush/rush00`,
`shells/shell01`.

## Como funciona

El panel izquierdo lanza `zenpack`, que lista los packs, deja crear uno nuevo y
guarda el elegido en `~/.local/state/zenidx/current-pack`. Los demas paneles no
reciben ninguna señal: **releen ese fichero en cada vuelta del `watch`**, asi que
cambiar de pack los reapunta a todos en un par de segundos.

## Comandos

| Comando | Que hace |
|---|---|
| `zenpack` | Menu para elegir o crear pack. `zenpack c03` selecciona directo, `zenpack -p` imprime el actual |
| `zenlayout` | Abre Terminator con el layout. `zenlayout c04` preselecciona pack |
| `zenin SUBDIR CMD...` | Ejecuta CMD dentro de `<pack>/SUBDIR` |
| `zenwatch [-n SEG] SUBDIR CMD...` | Panel de vigilancia: repite CMD en el pack; Ctrl+C deja shell viva ahi |
| `zentest [--compact]` | Compila y lanza el runner de tests del pack |
| `zenshell [SUBDIR]` | Shell interactiva dentro del pack |
| `zcd [SUBDIR]` | Salta al pack desde cualquier shell (funcion de zsh) |

## Instalacion

```sh
./install.sh
```

Copia `bin/*` a `~/.local/bin`, instala el layout de Terminator (con backup del
config previo), crea `~/.zshenv` y añade `zcd` a `~/.zshrc`. Es idempotente.

## Tres cosas que costaron encontrar

**1. `~/.zshrc` no vale para el PATH.** Terminator lanza el comando de cada panel
con `zsh -c`, que es no interactivo: zsh no lee `~/.zshrc`. Si el PATH se monta
ahi, los paneles no encuentran nada de `~/.local/bin` (ni `zenpack`, ni siquiera
`claude`) y el layout arranca en silencio con shells pelados. Por eso el PATH
vive en `~/.zshenv`, que zsh lee **siempre**.

**2. Ctrl+C cerraba el panel entero.** Con `command = "watch ...; exec $SHELL"`,
Terminator arranca `zsh -c "..."` y Ctrl+C va a todo el grupo de procesos en
primer plano: mataba el `watch` *y* el `zsh` padre, asi que el panel se cerraba
en vez de dejar una shell. `zenwatch` lo resuelve con `trap ':' INT` — un trap
con comando (no con `""`) se resetea a por-defecto en el hijo, asi que el
`watch` recibe la señal y el script no. Los paneles usan `exec zenwatch` para
que no quede ningun `zsh` padre por medio.

**3. Terminator reescribe su config.** Al cerrarse guarda `~/.config/terminator/config`
con configobj, y en el proceso **borra todos los comentarios**. Tampoco
interpreta `\"` dentro de un valor entrecomillado: las comillas escapadas llegan
literales al shell. De ahi que los comandos de los paneles sean cortos y la
logica viva en los scripts.
