# mazegen

Reusable random maze generator, born in the 42 project **A-Maze-ing**.

It builds rectangular mazes with a "42" drawn by fully closed cells in the
middle, either **perfect** (one single path between any two cells) or as a
**Pac-Man-like board** (fully connected, several loops, no dead-ends when the
size allows it). Generation is reproducible through a seed.

## Install

```sh
pip install mazegen-1.0.0-py3-none-any.whl
```

## Basic example

```python
from mazegen import MazeGenerator

gen = MazeGenerator(20, 15, entry=(0, 0), exit=(19, 14), perfect=True)
grid = gen.generate()            # grid[y][x] -> closed walls of cell (x, y)
print(gen.seed)                  # seed that was used (random if not given)
print(gen.solution_directions()) # shortest path, e.g. "EESSWS..."
```

## Custom parameters

| Parameter | Default | Meaning |
|---|---|---|
| `width`, `height` | - | size in cells |
| `entry`, `exit` | `(0, 0)`, bottom-right | cells as `(x, y)` |
| `perfect` | `False` | `True`: perfect maze; `False`: Pac-Man board |
| `seed` | random | same seed + same parameters = same maze |
| `algorithm` | `"backtracker"` | `"backtracker"` or `"prim"` |
| `pattern` | `True` | draw the "42" (omitted if the maze is smaller than 9x7) |

Invalid parameters raise `mazegen.MazeError` (a `ValueError`).
`gen.generate(seed=123)` regenerates the maze with a new seed.

## Accessing the maze

- `gen.grid[y][x]`: bitmask of the **closed** walls of cell `(x, y)`:
  `NORTH = 1`, `EAST = 2`, `SOUTH = 4`, `WEST = 8` (also exported).
- `gen.is_open((x, y), mazegen.EAST)`: is that wall open?
- `gen.neighbours((x, y))`: cells reachable in one step.
- `gen.pattern_cells`: set of the fully closed cells drawing the "42";
  `gen.pattern_omitted` tells if it did not fit.
- `gen.solution()`: shortest path as a list of `(x, y)` cells.
- `gen.solution_directions()`: the same path as `N`/`E`/`S`/`W` letters.
- `gen.to_hex()`: one string of hexadecimal digits per row.

## License

MIT, see `LICENSE.md`.
