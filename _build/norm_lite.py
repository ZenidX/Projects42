#!/usr/bin/env python3
"""Chequeo mecanico tipo norminette (subset) sobre los .c generados.
No reemplaza a norminette; caza los fallos mas comunes de un generador:
ancho de columna, indentacion con tabs, longitud de funcion, nº funciones,
header, keywords prohibidas y return con parentesis."""
import os
import re
import sys

ROOT = "/mnt/e/WORK/Xavi/Projects42/c"
MODS = ["c02", "c03", "c04"]
FORBIDDEN = ["for", "switch", "goto"]


def width(line):
    col = 0
    for ch in line:
        if ch == "\t":
            col = (col // 4 + 1) * 4
        else:
            col += 1
    return col


def check(path):
    errs = []
    with open(path, "r", newline="") as f:
        raw = f.read()
    if "\r" in raw:
        errs.append("CRLF/CR presente")
    lines = raw.split("\n")
    # header: 11 lineas de comentario
    if len(lines) < 11 or not lines[0].startswith("/* ****"):
        errs.append("header 42 ausente")
    body = lines[11:]
    depth = 0
    func_lines = 0
    in_func = False
    nfunc = 0
    for n, line in enumerate(body, start=12):
        if width(line) > 80:
            errs.append(f"L{n}: >80 cols ({width(line)})")
        # indentacion: los espacios de sangria estan prohibidos
        indent = re.match(r"^[ \t]*", line).group(0)
        if " " in indent:
            errs.append(f"L{n}: espacio en la indentacion")
        if line != line.rstrip():
            errs.append(f"L{n}: espacio/tab al final")
        for kw in FORBIDDEN:
            if re.search(r"(^|[^\w])" + kw + r"([^\w]|$)", line):
                errs.append(f"L{n}: keyword prohibida '{kw}'")
        if re.search(r"\bdo\b\s*\{", line):
            errs.append(f"L{n}: do-while prohibido")
        s = line.strip()
        if s.startswith("return") and s not in ("return", "return;"):
            if not re.match(r"^return\s*\(", s) and s != "return ;":
                errs.append(f"L{n}: return sin parentesis: {s}")
        # conteo de funciones y lineas por funcion (heuristico por llaves)
        opens = line.count("{")
        closes = line.count("}")
        if not in_func and opens > 0 and depth == 0:
            in_func = True
            nfunc += 1
            func_lines = 0
        elif in_func and depth > 0:
            func_lines += 1
        depth += opens - closes
        if in_func and depth == 0:
            if func_lines > 25:
                errs.append(f"funcion #{nfunc} >25 lineas ({func_lines})")
            in_func = False
    if nfunc > 5:
        errs.append(f">5 funciones ({nfunc})")
    return errs


def main():
    total = 0
    bad = 0
    for mod in MODS:
        base = os.path.join(ROOT, mod)
        for ex in sorted(os.listdir(base)):
            d = os.path.join(base, ex)
            if not os.path.isdir(d):
                continue
            for fn in sorted(os.listdir(d)):
                if not fn.endswith(".c"):
                    continue
                path = os.path.join(d, fn)
                total += 1
                errs = check(path)
                if errs:
                    bad += 1
                    print(f"[KO] {mod}/{ex}/{fn}")
                    for e in errs:
                        print(f"      - {e}")
                else:
                    print(f"[OK] {mod}/{ex}/{fn}")
    print(f"\n{total} ficheros, {bad} con avisos")
    sys.exit(1 if bad else 0)


main()
