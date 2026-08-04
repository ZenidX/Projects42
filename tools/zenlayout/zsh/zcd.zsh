# --- zenidx ---------------------------------------------------------------
# Salta al pack de ejercicios activo (el que eligio `zenpack`).
#   zcd        -> raiz del pack        zcd repo   -> <pack>/repo
zcd() {
    local pack
    pack=$(zenpack --print) || { print -u2 "sin pack: elige uno con zenpack"; return 1; }
    cd "${pack}${1:+/$1}"
}
