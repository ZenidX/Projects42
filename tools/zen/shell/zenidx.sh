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
    pack=$(command zenpack --print) || {
        printf 'sin pack: elige uno con zenpack\n' >&2
        return 1
    }
    cd "${pack}${1:+/$1}" || return 1
}

# --- zenpack ---------------------------------------------------------------
# Envoltorio del script: cuando eliges (o creas) un pack, deja ademas esta
# shell dentro de su repo/. El script no puede hacerlo solo, por lo mismo que
# zcd es una funcion: su cd moriria con el proceso hijo.
#
# Solo se mueve si el pack se ha llegado a elegir: se compara la marca de
# tiempo del fichero de estado antes y despues, asi salir del menu con `q` o
# con Ctrl-C deja la shell donde estaba.
zenpack() {
    case "${1-}" in
        -p|--print|-h|--help)
            command zenpack "$@"
            return $?
        ;;
    esac

    local state before after pack
    state="${XDG_STATE_HOME:-$HOME/.local/state}/zenidx/current-pack"
    before=$(stat -c %.9Y "$state" 2>/dev/null || stat -c %Y "$state" 2>/dev/null || printf 'x')

    command zenpack "$@" || return $?

    after=$(stat -c %.9Y "$state" 2>/dev/null || stat -c %Y "$state" 2>/dev/null || printf 'x')
    [ "$before" = "$after" ] && return 0

    pack=$(command zenpack --print) || return 0
    if [ -d "$pack/repo" ]; then
        cd "$pack/repo" || return 1
    else
        cd "$pack" || return 1
    fi
}
