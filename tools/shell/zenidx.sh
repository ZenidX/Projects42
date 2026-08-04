# zenidx - integracion de shell. Lo instala tools/install.sh y lo cargan
# ~/.zshenv (zsh) o ~/.bashrc (bash).
#
# Va en .zshenv y no en .zshrc a proposito: los paneles de Terminator arrancan
# la shell en modo no interactivo (`zsh -c ...`), que NO lee .zshrc. Si el PATH
# viviera alli, los paneles no encontrarian zenwatch ni zentest.

# --- PATH ------------------------------------------------------------------
# El guard evita duplicados en shells anidadas.
case ":$PATH:" in
    *":$HOME/.local/bin:"*) ;;
    *) export PATH="$HOME/.local/bin:$PATH" ;;
esac

# --- raiz de los packs -----------------------------------------------------
# Cambiala si guardas los ejercicios en otro sitio.
: "${ZENIDX_ROOT:=$HOME/zenidx/packs}"
export ZENIDX_ROOT

# --- zcd -------------------------------------------------------------------
# Salta al pack activo. Es una funcion y no un script porque un `cd` en un
# proceso hijo no afectaria a tu shell.
#
#   zcd        -> raiz del pack        zcd repo   -> <pack>/repo
zcd() {
    local pack
    pack=$(zenpack --print) || {
        printf 'sin pack: elige uno con zenpack\n' >&2
        return 1
    }
    cd "${pack}${1:+/$1}" || return 1
}
