#!/usr/bin/env bash
# install.sh - instala el espacio de trabajo zenidx.
#
#   ./install.sh           instala
#   ./install.sh --dry-run ensena lo que haria, sin tocar nada
#
# Es idempotente: puedes relanzarlo para actualizar. Todo lo que sobrescribe
# se guarda antes con sufijo .bak.
set -euo pipefail

SRC_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

BIN_DIR="$HOME/.local/bin"
SHARE_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/zenidx"
CONF_DIR="${XDG_CONFIG_HOME:-$HOME/.config}/zenidx"
TERM_DIR="${XDG_CONFIG_HOME:-$HOME/.config}/terminator"
PACKS_DIR="${ZENIDX_ROOT:-$HOME/zenidx/packs}"

SCRIPTS=(zen zenpack zenin zenwatch zentest zenshell zenlayout zengit zennorm zenhelp)

DRY=0
case "${1-}" in
    -h|--help) sed -n '2,9p' "$0" | cut -c3-; exit 0 ;;
    --dry-run) DRY=1 ;;
    '')        ;;
    *)         printf 'install.sh: opcion desconocida %s\n' "$1" >&2; exit 2 ;;
esac

if [ -t 1 ]; then
    B=$'\033[1m'; D=$'\033[2m'; G=$'\033[32m'; Y=$'\033[33m'; RED=$'\033[31m'; R=$'\033[0m'
else
    B=''; D=''; G=''; Y=''; RED=''; R=''
fi

say()  { printf '  %s\n' "$*"; }
ok()   { printf '  %s+%s %s\n' "$G" "$R" "$*"; }
warn() { printf '  %s!%s %s\n' "$Y" "$R" "$*"; }
run()  { if [ "$DRY" -eq 1 ]; then printf '  %s$ %s%s\n' "$D" "$*" "$R"; else "$@"; fi; }

printf '\n%sInstalador del espacio de trabajo zenidx%s\n' "$B" "$R"
[ "$DRY" -eq 1 ] && printf '%s(dry-run: no se toca nada)%s\n' "$Y" "$R"
printf '\n'

# --- 1. dependencias -------------------------------------------------------

printf '%sDependencias%s\n' "$B" "$R"
missing=0
for c in terminator watch awk sed; do
    if command -v "$c" >/dev/null 2>&1; then
        ok "$c"
    else
        printf '  %s-%s %s %sfalta (imprescindible)%s\n' "$RED" "$R" "$c" "$RED" "$R"
        missing=1
    fi
done
for c in tree norminette cc git; do
    if command -v "$c" >/dev/null 2>&1; then
        ok "$c"
    else
        warn "$c falta (opcional: el panel que lo usa dira 'command not found')"
    fi
done
if [ "$missing" -eq 1 ]; then
    printf '\n%sFaltan dependencias imprescindibles.%s\n' "$RED" "$R"
    printf '%sEn Debian/Ubuntu: sudo apt install terminator procps gawk%s\n\n' "$D" "$R"
    exit 1
fi
printf '\n'

# --- 2. scripts ------------------------------------------------------------

printf '%sScripts%s -> %s\n' "$B" "$R" "$BIN_DIR"
run mkdir -p "$BIN_DIR"
for s in "${SCRIPTS[@]}"; do
    if [ ! -f "$SRC_DIR/bin/$s" ]; then
        printf '  %s-%s falta %s en el paquete\n' "$RED" "$R" "$s"
        exit 1
    fi
    if [ -f "$BIN_DIR/$s" ] && ! cmp -s "$SRC_DIR/bin/$s" "$BIN_DIR/$s"; then
        run cp -f "$BIN_DIR/$s" "$BIN_DIR/$s.bak"
        ok "$s (actualizado, copia previa en $s.bak)"
    else
        ok "$s"
    fi
    run install -m 755 "$SRC_DIR/bin/$s" "$BIN_DIR/$s"
done
printf '\n'

# --- 3. datos: plantillas --------------------------------------------------

printf '%sPlantillas%s -> %s\n' "$B" "$R" "$SHARE_DIR"
run mkdir -p "$SHARE_DIR/test"
run install -m 644 "$SRC_DIR/terminator/layouts.conf.in" "$SHARE_DIR/layouts.conf.in"
ok 'layouts.conf.in (plantilla de layouts de terminator)'
run install -m 644 "$SRC_DIR/template/test/Makefile" "$SHARE_DIR/test/Makefile"
ok 'test/Makefile (semilla para packs nuevos)'
run install -m 644 "$SRC_DIR/shell/zenidx.sh" "$SHARE_DIR/zenidx.sh"
ok 'zenidx.sh (PATH + funciones zcd y zenpack)'
printf '\n'

# --- 4. integracion de shell ----------------------------------------------

printf '%sIntegracion de shell%s\n' "$B" "$R"
LINE="[ -r \"$SHARE_DIR/zenidx.sh\" ] && . \"$SHARE_DIR/zenidx.sh\"  # zenidx"

# zsh: va en .zshenv porque los paneles de terminator arrancan `zsh -c`, que
# no lee .zshrc. bash: .bashrc es lo mas parecido que hay.
case "$(basename "${SHELL:-/bin/bash}")" in
    zsh)  RC="$HOME/.zshenv" ;;
    bash) RC="$HOME/.bashrc" ;;
    *)    RC="$HOME/.profile" ;;
esac

if [ -f "$RC" ] && grep -qF '# zenidx' "$RC" 2>/dev/null; then
    ok "$(basename "$RC") ya lo carga"
else
    if [ "$DRY" -eq 1 ]; then
        printf '  %s$ echo ... >> %s%s\n' "$D" "$RC" "$R"
    else
        printf '\n%s\n' "$LINE" >> "$RC"
    fi
    ok "anadido a $(basename "$RC")"
fi
printf '\n'

# --- 5. arbol de packs -----------------------------------------------------

printf '%sPacks%s\n' "$B" "$R"
if [ -d "$PACKS_DIR" ]; then
    n=$(find "$PACKS_DIR" -mindepth 2 -maxdepth 2 -type d 2>/dev/null | wc -l)
    ok "$PACKS_DIR ($n packs)"
else
    run mkdir -p "$PACKS_DIR"
    ok "$PACKS_DIR (creado, vacio)"
    say "${D}crea el primero con 'zenpack -n'${R}"
fi
printf '\n'

# --- 6. ajustes y layout de terminator -------------------------------------

printf '%sConfiguracion%s\n' "$B" "$R"
if [ -f "$CONF_DIR/config" ]; then
    ok "$CONF_DIR/config ya existe (no se toca)"
else
    run mkdir -p "$CONF_DIR"
    if [ "$DRY" -eq 0 ]; then
        cat > "$CONF_DIR/config" <<'EOF'
# Ajustes del espacio de trabajo zenidx. Los edita `zen`.
layout=default
n_tree=2
n_norm=5
n_test=2
assist=claude
EOF
    fi
    ok "$CONF_DIR/config (ajustes por defecto)"
fi

if [ -f "$TERM_DIR/config" ]; then
    warn "$TERM_DIR/config ya existe: se sustituye SOLO la seccion [layouts]"
    say "${D}tu perfil, keybindings y global_config se conservan${R}"
    say "${D}copia de seguridad en config.bak${R}"
fi
if [ "$DRY" -eq 1 ]; then
    printf '  %s$ zen --apply%s\n' "$D" "$R"
else
    if PATH="$BIN_DIR:$PATH" zen --apply; then
        ok 'layouts de terminator generados'
    else
        printf '  %s-%s zen --apply ha fallado\n' "$RED" "$R"
        exit 1
    fi
fi
printf '\n'

# --- 7. resumen ------------------------------------------------------------

printf '%sListo.%s\n\n' "$G" "$R"
if ! command -v zen >/dev/null 2>&1; then
    printf '  %s%s no esta en tu PATH todavia.%s\n' "$Y" "$BIN_DIR" "$R"
    printf '  Abre una terminal nueva, o ejecuta:  %s. %s/zenidx.sh%s\n\n' "$B" "$SHARE_DIR" "$R"
fi
printf '  %szenhelp%s      que se puede hacer aqui: el mapa completo\n' "$B" "$R"
printf '  %szen%s          menu de configuracion del espacio\n' "$B" "$R"
printf '  %szenlayout%s    abre terminator con el layout configurado\n' "$B" "$R"
printf '  %szenpack%s      elige o crea un pack de ejercicios\n' "$B" "$R"
printf '  %szentest%s      lanza los tests del pack activo\n' "$B" "$R"
printf '  %szcd%s          salta al pack activo (funcion de shell)\n\n' "$B" "$R"
