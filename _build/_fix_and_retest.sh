#!/bin/bash
set -u
ROOT=/mnt/e/WORK/Xavi/Projects42
S0=$ROOT/shells/shell00
S1=$ROOT/shells/shell01
TMP=$(mktemp -d)
PASS=0; FAIL=0
ok()  { echo "PASS: $1"; PASS=$((PASS+1)); }
bad() { echo "FAIL: $1"; FAIL=$((FAIL+1)); }

mkdir -p "$S0/ex01" "$S0/ex02"

# ex01: regenerar y copiar
cd "$TMP"
printf '%040d' 0 > testShell00
chmod 455 testShell00; touch -t 06012342 testShell00
tar -cf "$S0/ex01/testShell00.tar" testShell00
perm=$(tar -tvf "$S0/ex01/testShell00.tar" | awk '{print $1, $3}')
[ "$perm" = "-r--r-xr-x 40" ] && ok "s00/ex01 testShell00.tar en su sitio" || bad "s00/ex01: $perm"

# ex02: regenerar y copiar
mkdir -p "$TMP/ex02" && cd "$TMP/ex02"
mkdir test0 test2
printf 'abc\n' > test1
printf '1'    > test3
printf '2\n'  > test4
ln test3 test5
ln -s test0 test6
chmod 715 test0; chmod 714 test1; chmod 504 test2; chmod 404 test3; chmod 641 test4
touch -t 06012047 test0; touch -t 06012146 test1; touch -t 06012245 test2
touch -t 06012344 test3 test5; touch -t 06012343 test4; touch -h -t 06012220 test6
tar -cf "$S0/ex02/exo2.tar" *
n=$(tar -tvf "$S0/ex02/exo2.tar" | grep -c -E '^(drwx--xr-x .* test0/|-rwx--xr-- .* test1|dr-x---r-- .* test2/|[-h]r-----r-- .* test3|-rw-r----x .* test4|[-h]r-----r-- .* test5|lrwxrwxrwx .* test6 -> test0)')
[ "$n" = 7 ] && ok "s00/ex02 exo2.tar: 7/7 entradas correctas" || { bad "s00/ex02: $n/7"; tar -tvf "$S0/ex02/exo2.tar"; }

# s01/ex04: probar la logica awk contra salida real de ifconfig (sample) e instalar net-tools si se puede
cat > "$TMP/ifconfig_sample" <<'EOF'
eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.1.50  netmask 255.255.255.0  broadcast 192.168.1.255
        inet6 fe80::215:5dff:fe22:1a01  prefixlen 64  scopeid 0x20<link>
        ether 00:15:5d:22:1a:01  txqueuelen 1000  (Ethernet)
lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        loop  txqueuelen 1000  (Bucle local)
wlan0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        ether a4:b1:c2:d3:e4:f5  txqueuelen 1000  (Ethernet)
EOF
out=$(awk '/ether/ {print $2}' "$TMP/ifconfig_sample" | tr '\n' ' ')
[ "$out" = "00:15:5d:22:1a:01 a4:b1:c2:d3:e4:f5 " ] && ok "s01/ex04 logica MAC.sh valida (sobre salida ifconfig real)" || bad "s01/ex04: $out"

# s01/ex07: rango dentro del passwd de WSL
export FT_LINE1=2 FT_LINE2=5
out=$(sh "$S1/ex07/r_dwssap.sh")
echo "--- r_dwssap(2..5): $out"
ncom=$(printf '%s' "$out" | tr -cd ',' | wc -c)
case "$out" in
  *.) [ "$ncom" = 3 ] && ok "s01/ex07 4 nombres, separador ', ', punto final" || bad "s01/ex07 comas=$ncom";;
  *) bad "s01/ex07 sin punto final";;
esac
# ademas: verificar orden inverso
first=$(printf '%s' "$out" | cut -d, -f1)
second=$(printf '%s' "$out" | cut -d, -f2 | sed 's/^ //')
[ "$(printf '%s\n%s\n' "$first" "$second" | sort -r | head -1)" = "$first" ] && ok "s01/ex07 orden alfabetico inverso" || bad "s01/ex07 orden"

# s01/ex05: reintentar el tar con debugging
mkdir -p "$TMP/s1ex05" && cd "$TMP/s1ex05"
sed 's/\r$//' "$S1/ex05/create_marvin.sh" > cm.sh
sh cm.sh
fname=$(find . -maxdepth 1 -name '*MaRViN*' -printf '%f\n')
echo "--- fichero creado: [$fname] ($(stat -c%s "./$fname") bytes, $(stat -c%A "./$fname"))"
rm -f "$S1/ex05/ex05.tar"
tar -cf "$S1/ex05/ex05.tar" -- "$fname" && ok "s01/ex05 ex05.tar generado" || bad "s01/ex05 tar"
tar -tvf "$S1/ex05/ex05.tar"

echo
echo "==== RESULTADO: $PASS PASS / $FAIL FAIL ===="
rm -rf "$TMP"
