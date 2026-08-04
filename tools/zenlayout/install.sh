#!/usr/bin/env bash
# Instala zenlayout en esta maquina. Idempotente: se puede relanzar sin miedo.
set -euo pipefail

here=$(cd -- "$(dirname -- "$0")" && pwd)
bindir="$HOME/.local/bin"
tconf="$HOME/.config/terminator/config"

echo "==> scripts en $bindir"
mkdir -p "$bindir"
install -m 755 "$here"/bin/* "$bindir/"

echo "==> ~/.zshenv (PATH para shells no interactivas: los paneles usan zsh -c)"
if [ -e "$HOME/.zshenv" ] && ! cmp -s "$here/zsh/zshenv" "$HOME/.zshenv"; then
    cp "$HOME/.zshenv" "$HOME/.zshenv.bak.$(date +%Y%m%d-%H%M%S)"
    echo "    guardado backup del .zshenv anterior"
fi
cp "$here/zsh/zshenv" "$HOME/.zshenv"

echo "==> funcion zcd en ~/.zshrc"
if grep -q '^zcd()' "$HOME/.zshrc" 2>/dev/null; then
    echo "    ya estaba, no toco nada"
else
    cat "$here/zsh/zcd.zsh" >> "$HOME/.zshrc"
fi

echo "==> layout de Terminator en $tconf"
mkdir -p "$(dirname "$tconf")"
if [ -e "$tconf" ]; then
    cp "$tconf" "$tconf.bak.$(date +%Y%m%d-%H%M%S)"
    echo "    guardado backup del config anterior"
fi
# El config guarda rutas absolutas en los `directory`; se reescriben al $HOME
# de esta maquina para que el paquete sirva tal cual en otra cuenta.
sed "s|/home/xalara|$HOME|g" "$here/terminator/config" > "$tconf"

cat <<'EOF'

Listo. Dos avisos:

  - El PATH ahora se monta en ~/.zshenv; si lo tenias tambien en ~/.zshrc,
    quitalo de ahi para no prependerlo dos veces.
  - Terminator carga el config al arrancar, asi que hay que cerrarlo del todo
    y volver a abrirlo (`zenlayout`) para estrenar el layout.
EOF
