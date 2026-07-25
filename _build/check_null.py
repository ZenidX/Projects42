import re
import sys

src = open(sys.argv[1], encoding="utf-8").read()
for m in re.finditer(r'F\["([^"]+)"\]\s*=\s*"""(.*?)"""', src, re.S):
    rel, body = m.group(1), m.group(2)
    if not rel.endswith(".c"):
        continue
    if "NULL" in body and not any(
        h in body for h in ("<stddef.h>", "<stdlib.h>", "<unistd.h>", "<stdio.h>")
    ):
        print("SIN INCLUDE:", rel)
print("barrido completo")
