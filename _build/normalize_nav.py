#!/usr/bin/env python3
"""Unifica el <nav> de todas las paginas de web/ y valida enlaces internos."""
import os
import re
import sys

WEB = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "web")

PAGES = [("index.html", "Trasfondo"), ("shell00.html", "Shell 00"), ("shell01.html", "Shell 01")]
PAGES += [(f"c{i:02d}.html", f"C {i:02d}") for i in range(14)]

def nav_for(current):
    links = []
    for fname, label in PAGES:
        cls = ' class="active"' if fname == current else ""
        links.append(f'      <a href="{fname}"{cls}>{label}</a>')
    return "<nav>\n" + "\n".join(links) + "\n    </nav>"

errors = []
for fname, _ in PAGES:
    path = os.path.join(WEB, fname)
    if not os.path.exists(path):
        errors.append(f"FALTA {fname}")
        continue
    with open(path, encoding="utf-8") as f:
        html = f.read()
    new_html, n = re.subn(r"<nav>.*?</nav>", nav_for(fname), html, count=1, flags=re.S)
    if n != 1:
        errors.append(f"{fname}: no se encontro <nav>")
        continue
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(new_html)
    # validar hrefs internos
    for href in re.findall(r'href="([^"#]+\.html)"', new_html):
        if not os.path.exists(os.path.join(WEB, href)):
            errors.append(f"{fname}: enlace roto -> {href}")
    print(f"OK {fname}")

if errors:
    print("\n".join(["", "ERRORES:"] + errors))
    sys.exit(1)
print("\nnav unificado en", len(PAGES), "paginas")
