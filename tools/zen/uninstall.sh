#!/usr/bin/env bash
# uninstall.sh - quita el espacio de trabajo zenidx.
#
#   ./uninstall.sh          quita scripts, plantillas y ajustes
#   ./uninstall.sh --all    ademas restaura el config de terminator del .bak
#
# NUNCA toca ~/zenidx/packs: tus ejercicios se quedan donde estan.
set -euo pipefail

BIN_DIR="$HOME/.local/bin"
SHARE_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/zenidx"
CONF_DIR="${XDG_CONFIG_HOME:-$HOME/.config}/zenidx"
TERM_CFG="${XDG_CONFIG_HOME:-$HOME/.config}/terminator/config"
STATE_DIR="${XDG_STATE_HOME:-$HOME/.local/state}/zenidx"

SCRIPTS=(zen zenpack zenin zenwatch zentest zenshell zenlayout zengit)

ALL=0
case "${1-}" in
    -h|--help) sed -n '2,9p' "$0" | cut -c3-; exit 0 ;;
    --all)     ALL=1 ;;
    '')        ;;
    *)         printf 'uninstall.sh: opcion desconocida %s\n' "$1" >&2; exit 2 ;;
esac

if [ -t 1 ]; then
    B=$'\033[1m'; D=$'\033[2m'; G=$'\033[32m'; Y=$'\033[33m'; R=$'\033[0m'
else
    B=''; D=''; G=''; Y=''; R=''
fi

printf '\n%sDesinstalando zenidx%s\n\n' "$B" "$R"

for s in "${SCRIPTS[@]}"; do
    rm -f "$BIN_DIR/$s" "$BIN_DIR/$s.bak"
done
printf '  %s+%s scripts quitados de %s\n' "$G" "$R" "$BIN_DIR"

rm -rf "$SHARE_DIR"
printf '  %s+%s plantillas quitadas de %s\n' "$G" "$R" "$SHARE_DIR"

rm -rf "$CONF_DIR" "$STATE_DIR"
printf '  %s+%s ajustes y estado quitados\n' "$G" "$R"

if [ "$ALL" -eq 1 ] && [ -f "$TERM_CFG.bak" ]; then
    mv -f "$TERM_CFG.bak" "$TERM_CFG"
    printf '  %s+%s config de terminator restaurado del .bak\n' "$G" "$R"
elif [ "$ALL" -eq 1 ]; then
    printf '  %s!%s no hay %s.bak que restaurar\n' "$Y" "$R" "$TERM_CFG"
else
    printf '  %s!%s el config de terminator se queda como esta\n' "$Y" "$R"
    printf '    %s(--all lo restaura desde config.bak)%s\n' "$D" "$R"
fi

printf '\n  %sQueda por quitar a mano:%s\n' "$B" "$R"
printf '    la linea con %s# zenidx%s de tu .zshenv / .bashrc / .profile\n' "$B" "$R"
printf '\n  %sTus ejercicios en ~/zenidx/packs NO se han tocado.%s\n\n' "$D" "$R"
