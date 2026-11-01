#!/usr/bin/env python3
"""A-Maze-ing runner for zentest: ./runner <repo>.

Checks the files, the linters, the error handling of a_maze_ing.py, and
generates many mazes that are verified with the subject's maze_analyzer.py.
"""

import os
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ANALYZER = os.path.join(HERE, "..", "extra", "maze_analyzer.py")
GREEN, RED, RESET = "\033[32m", "\033[31m", "\033[0m"
results = {"ok": 0, "ko": 0}


def report(ok, name, detail=""):
    results["ok" if ok else "ko"] += 1
    tag = f"{GREEN}[OK]{RESET}" if ok else f"{RED}[KO]{RESET}"
    print(f"{tag} {name}")
    if detail and not ok:
        for line in detail.strip().splitlines()[:6]:
            print(f"     {line}")


def section(title):
    print(f"\n── {title}")


def run(cmd, cwd, stdin=""):
    try:
        p = subprocess.run(cmd, cwd=cwd, input=stdin, capture_output=True,
                           text=True, timeout=60)
        return p.returncode, p.stdout + p.stderr
    except subprocess.TimeoutExpired:
        return -1, "TIMEOUT"


def config(**keys):
    base = {"WIDTH": "20", "HEIGHT": "15", "ENTRY": "0,0", "EXIT": "19,14",
            "OUTPUT_FILE": "out.txt", "PERFECT": "True"}
    base.update(keys)
    return "\n".join(f"{k}={v}" for k, v in base.items() if v is not None)


def main(repo):
    repo = os.path.abspath(repo)
    main_py = os.path.join(repo, "a_maze_ing.py")

    section("ficheros")
    for name in ("a_maze_ing.py", "config.txt", "Makefile", "README.md",
                 "LICENSE.md", ".gitignore", "pyproject.toml"):
        report(os.path.isfile(os.path.join(repo, name)), name)
    wheels = [f for f in os.listdir(repo) if f.startswith("mazegen-")
              and (f.endswith(".whl") or f.endswith(".tar.gz"))]
    report(bool(wheels), "paquete mazegen-* en la raíz")
    if os.path.isfile(os.path.join(repo, "README.md")):
        with open(os.path.join(repo, "README.md")) as f:
            first = f.readline().strip()
        report(first.startswith("*This project has been created as part of "
                                "the 42 curriculum by"), "README: 1ª línea")
    if not os.path.isfile(main_py):
        return

    section("lint")
    code, out = run(["flake8", ".", "--exclude", ".venv"], repo)
    report(code == 0, "flake8", out)
    code, out = run(["mypy", ".", "--warn-return-any", "--warn-unused-ignores",
                     "--ignore-missing-imports", "--disallow-untyped-defs",
                     "--check-untyped-defs", "--exclude", ".venv"], repo)
    report(code == 0, "mypy (flags del subject)", out)

    with tempfile.TemporaryDirectory() as tmp:
        cfg = os.path.join(tmp, "c.txt")

        section("errores (mensaje claro, sin traceback)")
        cases = [
            ("sin argumentos", None, []),
            ("fichero inexistente", None, ["nope.txt"]),
            ("línea sin =", "WIDTH 20", ["c.txt"]),
            ("falta una clave", config(PERFECT=None), ["c.txt"]),
            ("WIDTH no numérico", config(WIDTH="abc"), ["c.txt"]),
            ("WIDTH negativo", config(WIDTH="-3"), ["c.txt"]),
            ("ENTRY fuera", config(ENTRY="50,0"), ["c.txt"]),
            ("ENTRY mal escrito", config(ENTRY="0;0"), ["c.txt"]),
            ("ENTRY == EXIT", config(EXIT="0,0"), ["c.txt"]),
            ("PERFECT raro", config(PERFECT="maybe"), ["c.txt"]),
            ("ENTRY dentro del 42", config(ENTRY="6,5"), ["c.txt"]),
            ("salida no escribible", config(OUTPUT_FILE="/no/dir/x"),
             ["c.txt"]),
        ]
        for name, text, args in cases:
            if text is not None:
                with open(cfg, "w") as f:
                    f.write(text + "\n")
            code, out = run([sys.executable, main_py, *args], tmp, "5\n")
            report("Traceback" not in out and code != 0 and out.strip(),
                   name, out)

        section("mazes verificados con maze_analyzer.py")
        sizes = [(9, 7), (10, 8), (20, 15), (21, 14), (40, 30), (5, 4)]
        for perfect in ("True", "False"):
            for w, h in sizes:
                bad = []
                for seed in range(5):
                    with open(cfg, "w") as f:
                        f.write(config(WIDTH=str(w), HEIGHT=str(h),
                                       EXIT=f"{w - 1},{h - 1}",
                                       SEED=str(seed), PERFECT=perfect))
                    code, out = run([sys.executable, main_py, "c.txt"], tmp,
                                    "5\n")
                    if code != 0 or "Traceback" in out:
                        bad.append(f"seed {seed}: {out[-200:]}")
                        continue
                    extra = [] if perfect == "True" else ["--max-dead-ends",
                                                          "2"]
                    _, verdict = run([sys.executable, ANALYZER,
                                      os.path.join(tmp, "out.txt"), *extra],
                                     tmp)
                    want = ("PERFECT" if perfect == "True"
                            else "Pac-Man-USABLE")
                    line = verdict.split("Verdict:")[-1].strip()
                    if not line.startswith(want):
                        bad.append(f"seed {seed}: {line}")
                mode = "perfect" if perfect == "True" else "pac-man"
                report(not bad, f"{w}x{h} {mode} (5 seeds)", "\n".join(bad))

        section("reproducibilidad")
        outs = []
        for _ in range(2):
            with open(cfg, "w") as f:
                f.write(config(SEED="1234"))
            run([sys.executable, main_py, "c.txt"], tmp, "5\n")
            with open(os.path.join(tmp, "out.txt")) as f:
                outs.append(f.read())
        report(outs[0] == outs[1], "misma SEED = mismo maze")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: ./runner <repo>")
        sys.exit(2)
    main(sys.argv[1])
    print(f"\n{results['ok']} OK, {results['ko']} KO")
    sys.exit(1 if results["ko"] else 0)
