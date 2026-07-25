#!/bin/bash
cd /mnt/e/WORK/Xavi/Projects42/_build || exit 1
for g in gen_c_files gen_c02_04 gen_c05_07 gen_c08_10 gen_c11_13; do
    echo "--- $g"
    python3 "$g.py" 2>&1 | tail -1
done
