*This project has been created as part of the 42 curriculum by xalara.*

# A-Maze-ing

## Description

A-Maze-ing is a maze generator written in Python. It reads a configuration
file, generates a random (but reproducible) maze, writes it to a file using a
hexadecimal wall encoding, and shows it in the terminal with colours and an
interactive menu.

Every maze has a **"42" drawn with fully closed cells** in its centre, and can
be generated in two modes:

- **Perfect** (`PERFECT=True`): exactly one path between the entry and the
  exit (no loops at all).
- **Pac-Man board** (`PERFECT=False`, the default): every corridor is
  reachable, the four corners and the centre are open, there are many
  independent loops and **no dead-end** (the bonus "braided" board), except the
  few cells enclosed by the "42" itself.

The generation logic is a standalone, pip-installable package, `mazegen`,
so it can be reused in a later project (Pac-Man).

### Files

| File | Role |
|---|---|
| `a_maze_ing.py` | main program: config → generation → output file → interactive display |
| `maze_config.py` | reads and validates the configuration file |
| `maze_output.py` | writes the hexadecimal output file |
| `maze_display.py` | terminal rendering (ANSI colours) |
| `mazegen/` | the reusable package (`MazeGenerator` class) |
| `mazegen-1.0.0-py3-none-any.whl` | the built package |
| `pyproject.toml` | everything needed to rebuild the package |
| `config.txt` | default configuration |
| `LICENSE.md` | MIT license of the reusable code |

## Instructions

Requirements: Python 3.10 or later.

```sh
make install        # flake8, mypy and build
make run            # python3 a_maze_ing.py config.txt
make debug          # same, under pdb
make lint           # flake8 + mypy with the subject's flags
make lint-strict    # flake8 + mypy --strict
make build          # rebuild mazegen-1.0.0-py3-none-any.whl
make clean          # remove caches and build leftovers
```

Or directly:

```sh
python3 a_maze_ing.py config.txt
```

The program writes the output file, draws the maze and shows a menu:

```
1. Re-generate a new maze          (new random seed, output file rewritten)
2. Show/Hide path from entry to exit
3. Change wall colour              (white, blue, green, magenta, orange, grey)
4. Change '42' colour              (yellow, red, cyan, pink, same as walls)
5. Quit
```

Every error (missing file, bad syntax, unknown key, coordinates out of the
maze, entry inside the "42", impossible Pac-Man board, unwritable output...)
is reported with a clear message and the program exits with status 1, never
with a traceback.

To check an output file with `maze_analyzer.py` (given with the subject,
not part of this repository):

```sh
python3 maze_analyzer.py maze.txt                    # perfect / playable
python3 maze_analyzer.py maze.txt --max-dead-ends 0  # no-dead-end bonus
```

### Configuration file

One `KEY=VALUE` per line. Empty lines and lines starting with `#` are
ignored. Keys are case-insensitive, spaces around `=` are allowed. An
unknown or repeated key is an error (it catches typos).

| Key | Mandatory | Value | Example |
|---|---|---|---|
| `WIDTH` | yes | number of columns, 1 to 200 | `WIDTH=20` |
| `HEIGHT` | yes | number of rows, 1 to 200 | `HEIGHT=15` |
| `ENTRY` | yes | entry cell `x,y` (x = column, y = row, from 0) | `ENTRY=0,0` |
| `EXIT` | yes | exit cell `x,y`, different from the entry | `EXIT=19,14` |
| `OUTPUT_FILE` | yes | path of the output file | `OUTPUT_FILE=maze.txt` |
| `PERFECT` | yes | `True`/`False` (also `1`/`0`, `yes`/`no`) | `PERFECT=False` |
| `SEED` | no | integer ≥ 0; same seed = same maze. Random if absent | `SEED=42` |
| `ALGORITHM` | no | `backtracker` (default) or `prim` | `ALGORITHM=prim` |

The "42" needs a maze of at least 9x7 cells; on a smaller maze it is omitted
and an error message is printed. A Pac-Man board needs room for two loops
(`(WIDTH - 1) * (HEIGHT - 1) >= 2`).

### Output file

One hexadecimal digit per cell, row by row. Each bit is a **closed** wall:
bit 0 North, bit 1 East, bit 2 South, bit 3 West (so `F` is a fully closed
cell, as in the "42"). After an empty line: the entry, the exit and the
shortest path in `N`/`E`/`S`/`W` letters.

```
95395539555515513913
...
C4545456C54546C54446

0,0
19,14
SSESSESSSSEEESENESSSSEEENNEEESEESSESEEE
```

(This is the output of the default `config.txt`.)

## Maze generation algorithm

1. **The "42"**: its cells (a 7x5 pattern) are marked as fully closed, centred
   in the grid. The middle column of the pattern is left open, so the centre
   of the board is always a corridor.
2. **Perfect maze — recursive backtracker** (randomized depth-first search),
   with an explicit stack instead of recursion: start on the entry, move to a
   random unvisited neighbour opening the wall between them, and go back when
   there is none. Every free cell is visited once and every opening joins a
   new cell, so the result is a spanning tree: connected, no loops, one path
   between any two cells. A 3x3 open area is impossible (it would be a loop).
   Optionally, **randomized Prim** (`ALGORITHM=prim`) grows the tree from a
   random frontier wall instead, which gives shorter, bushier corridors.
3. **Pac-Man board** (`PERFECT=False`): starting from the perfect maze,
   every dead-end gets one extra wall opened ("braiding"), preferably
   towards another dead-end, which fixes both with one wall. A wall is only
   opened if it does not create a 3x3 open area (corridors at most 2 wide).
   Each opening adds a loop, and if there are still fewer than 2 loops, random
   walls are opened until there are.

Walls are always opened and closed **on both sides at once**, so neighbouring
cells always agree, and the outer border is never opened.

### Why this algorithm

- The backtracker is the simplest algorithm that guarantees a perfect maze,
  and it gives long winding corridors, which look good and are hard to solve.
- Building the Pac-Man board **from** a perfect maze keeps full connectivity
  for free (we only ever add openings), and braiding removes exactly the
  thing a chased player hates: dead-ends.
- Everything is driven by one `random.Random(seed)`, so a seed always gives
  the same maze.

## Reusable code

The `mazegen` package (`mazegen/generator.py`) contains the single class
`MazeGenerator`, independent from the configuration file, the output file and
the display. It is built into `mazegen-1.0.0-py3-none-any.whl`:

```sh
python3 -m venv .venv && . .venv/bin/activate
pip install build && python3 -m build --wheel --outdir .   # rebuild
pip install mazegen-1.0.0-py3-none-any.whl                 # install
```

### Instantiate and use it

```python
from mazegen import MazeGenerator, MazeError, EAST

gen = MazeGenerator(20, 15, entry=(0, 0), exit=(19, 14),
                    perfect=True, seed=42, algorithm="backtracker")
grid = gen.generate()          # build it
gen.generate(seed=7)           # rebuild with another seed
```

### Custom parameters

`width`, `height`, `entry`, `exit` (cells as `(x, y)`), `perfect`
(default `False`), `seed` (default random, then readable in `gen.seed`),
`algorithm` (`"backtracker"` or `"prim"`) and `pattern` (draw the "42",
default `True`). Invalid parameters raise `MazeError` (a `ValueError`).

### Access the structure and a solution

- `gen.grid[y][x]`: closed walls of cell `(x, y)` as a bitmask
  (`NORTH = 1`, `EAST = 2`, `SOUTH = 4`, `WEST = 8`).
- `gen.is_open((x, y), EAST)`, `gen.neighbours((x, y))`.
- `gen.pattern_cells`: the cells of the "42"; `gen.pattern_omitted`.
- `gen.solution()`: shortest path from entry to exit, list of `(x, y)`.
- `gen.solution_directions()`: the same path as `"SSES..."`.
- `gen.to_hex()`: rows of hexadecimal digits.

The same documentation is shipped inside the package (`mazegen/README.md`).

## Advanced features

- No dead-end at all on Pac-Man boards (checked with
  `maze_analyzer.py --max-dead-ends 0`) — bonus.
- Two generation algorithms, `backtracker` and `prim` — bonus.
- Configurable "42" colour, including "same as walls".

## Team and project management

- **Team**: xalara, alone, so every role (design, code, tests, docs).
- **Planning**: first the generator and the output file, checked with the
  provided `maze_analyzer.py`; then the config parser and its errors; then
  the display; finally the package, license and README. The Pac-Man mode took
  longer than planned: the first idea (opening random walls) left dead-ends
  and could create 3x3 rooms, so it became braiding with a 3x3 check.
- **What worked well**: validating hundreds of mazes (every size from 2x2 to
  40x30, both modes, both algorithms, several seeds) with the analyzer in a
  loop caught the impossible 2x2 Pac-Man board, which is now an error.
- **What could be improved**: an MLX graphical display and an animation of
  the generation.
- **Tools**: flake8, mypy (`--strict`), `python -m build`, pdb, the subject's
  `maze_analyzer.py`, git.

## Resources

- [Maze generation algorithm — Wikipedia](https://en.wikipedia.org/wiki/Maze_generation_algorithm)
- Jamis Buck, *Mazes for Programmers* and his
  [blog series on maze algorithms](https://weblog.jamisbuck.org/2011/2/7/maze-generation-algorithm-recap)
  (backtracker, Prim, braiding)
- [Python `random` — reproducibility with seeds](https://docs.python.org/3/library/random.html)
- [Python Packaging User Guide](https://packaging.python.org/en/latest/tutorials/packaging-projects/)
- [ANSI escape codes — 256 colours](https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit)
- [Choose a License — MIT](https://choosealicense.com/licenses/mit/)

### AI usage

Claude (Anthropic) was used to write a first version of the whole project
(generator, config parser, display, packaging, README) and a script that
generates hundreds of mazes and checks them with `maze_analyzer.py`. All
of it was then reviewed, run and tested by hand before submission.
