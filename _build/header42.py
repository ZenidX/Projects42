"""Header 42 estandar (80 cols exactas). Uso: from header42 import header"""

LOGIN = "xalara"
EMAIL = "xalara@student.42barcelona.com"
DATE = "2026/07/23 12:00:00"

TPL = [
    "/* ************************************************************************** */",
    "/*                                                                            */",
    "/*                                                        :::      ::::::::   */",
    "/*                                                      :+:      :+:    :+:   */",
    "/*                                                    +:+ +:+         +:+     */",
    "/*                                                  +#+  +:+       +#+        */",
    "/*                                                +#+#+#+#+#+   +#+           */",
    "/*                                                     #+#    #+#             */",
    "/*                                                    ###   ########.fr       */",
    "/*                                                                            */",
    "/* ************************************************************************** */",
]


def _splice(line, text):
    out = "/*   " + text + line[5 + len(text):]
    assert len(out) == 80, f"header line !=80: {len(out)} [{text}]"
    return out


def header(fname):
    l = TPL[:]
    l[3] = _splice(l[3], fname)
    l[5] = _splice(l[5], f"By: {LOGIN} <{EMAIL}>")
    l[7] = _splice(l[7], f"Created: {DATE} by {LOGIN}")
    l[8] = _splice(l[8], f"Updated: {DATE} by {LOGIN}")
    return "\n".join(l) + "\n\n"


def write_file(root, rel, body):
    """Escribe root/rel con header + body (LF)."""
    import os
    path = os.path.join(root, rel)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w", newline="\n") as f:
        f.write(header(os.path.basename(rel)) + body)
    print(f"OK {rel}")
