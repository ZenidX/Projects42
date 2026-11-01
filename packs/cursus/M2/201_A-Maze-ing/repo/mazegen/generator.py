"""Reusable maze generator.

The whole generation logic lives in :class:`MazeGenerator`. A maze is a
rectangular grid of cells; every cell stores its closed walls as a 4-bit
integer (bit 0 = North, 1 = East, 2 = South, 3 = West), exactly like the
hexadecimal output format of A-Maze-ing.

Example:
    >>> from mazegen import MazeGenerator
    >>> gen = MazeGenerator(20, 15, entry=(0, 0), exit=(19, 14), seed=42)
    >>> grid = gen.generate()
    >>> gen.solution_directions()[:4]
    'SSES'
"""

from __future__ import annotations

import random
from collections import deque
from typing import Dict, List, Optional, Set, Tuple

Cell = Tuple[int, int]
"""A cell position as ``(x, y)``: x is the column, y is the row."""

NORTH = 1
EAST = 2
SOUTH = 4
WEST = 8
ALL_WALLS = NORTH | EAST | SOUTH | WEST

DIRECTIONS: Tuple[int, ...] = (NORTH, EAST, SOUTH, WEST)
STEP: Dict[int, Cell] = {
    NORTH: (0, -1),
    EAST: (1, 0),
    SOUTH: (0, 1),
    WEST: (-1, 0),
}
OPPOSITE: Dict[int, int] = {
    NORTH: SOUTH,
    EAST: WEST,
    SOUTH: NORTH,
    WEST: EAST,
}
LETTER: Dict[int, str] = {NORTH: "N", EAST: "E", SOUTH: "S", WEST: "W"}

ALGORITHMS: Tuple[str, ...] = ("backtracker", "prim")

# The "42" drawn with fully closed cells ('#'), 7 columns x 5 rows.
PATTERN_42: Tuple[str, ...] = (
    "#.#.###",
    "#.#...#",
    "###.###",
    "..#.#..",
    "..#.###",
)
PATTERN_WIDTH = len(PATTERN_42[0])
PATTERN_HEIGHT = len(PATTERN_42)


class MazeError(ValueError):
    """Raised when the maze parameters make generation impossible."""


class MazeGenerator:
    """Generate random, reproducible mazes with a "42" in the middle.

    Args:
        width: Number of columns (cells).
        height: Number of rows (cells).
        entry: Entry cell as ``(x, y)``.
        exit: Exit cell as ``(x, y)``.
        perfect: ``True`` for a perfect maze (exactly one path between any
            two cells). ``False`` (default) for a Pac-Man-like board: fully
            connected, with several loops and no dead-ends when possible.
        seed: Seed of the random generator. ``None`` picks a random one,
            readable afterwards in :attr:`seed`.
        algorithm: ``"backtracker"`` (recursive backtracker, default) or
            ``"prim"`` (randomized Prim).
        pattern: Draw the "42" pattern if the maze is large enough.

    Raises:
        MazeError: If a parameter is invalid.

    Attributes:
        grid: ``grid[y][x]`` is the wall bitmask of cell ``(x, y)``.
        pattern_cells: Fully closed cells drawing the "42".
        pattern_omitted: ``True`` if the "42" was requested but the maze is
            too small to hold it.
    """

    def __init__(
        self,
        width: int,
        height: int,
        entry: Cell = (0, 0),
        exit: Optional[Cell] = None,
        perfect: bool = False,
        seed: Optional[int] = None,
        algorithm: str = "backtracker",
        pattern: bool = True,
    ) -> None:
        """Validate the parameters; call :meth:`generate` to build."""
        if width < 1 or height < 1:
            raise MazeError("width and height must be at least 1")
        if width * height < 2:
            raise MazeError("the maze needs at least 2 cells")
        if not perfect and (width - 1) * (height - 1) < 2:
            raise MazeError(
                "a non-perfect (Pac-Man) board needs room for at least two "
                "loops: use a bigger maze or PERFECT=True"
            )
        if algorithm not in ALGORITHMS:
            raise MazeError(
                f"unknown algorithm '{algorithm}' "
                f"(available: {', '.join(ALGORITHMS)})"
            )
        self.width: int = width
        self.height: int = height
        self.entry: Cell = entry
        self.exit: Cell = exit if exit is not None else (width - 1,
                                                         height - 1)
        self.perfect: bool = perfect
        self.algorithm: str = algorithm
        self.seed: int = (seed if seed is not None
                          else random.randrange(2 ** 32))
        self.want_pattern: bool = pattern
        self.grid: List[List[int]] = []
        self.pattern_cells: Set[Cell] = set()
        self.pattern_omitted: bool = False
        self._rng: random.Random = random.Random(self.seed)
        self._check_endpoints()

    # ------------------------------------------------------------------ #
    # Public API
    # ------------------------------------------------------------------ #
    def generate(self, seed: Optional[int] = None) -> List[List[int]]:
        """Build a new maze and return its grid.

        Args:
            seed: New seed to use. ``None`` reuses :attr:`seed`, so calling
                ``generate()`` twice gives the same maze.

        Returns:
            The wall grid, ``grid[y][x]``.
        """
        if seed is not None:
            self.seed = seed
        self._rng = random.Random(self.seed)
        self.grid = [[ALL_WALLS] * self.width for _ in range(self.height)]
        self._place_pattern()
        if self.algorithm == "prim":
            self._carve_prim()
        else:
            self._carve_backtracker()
        if not self.perfect:
            self._braid()
            self._add_loops(2)
        return self.grid

    def solution(self) -> List[Cell]:
        """Return the shortest path from entry to exit, both included."""
        self._require_grid()
        previous: Dict[Cell, Optional[Cell]] = {self.entry: None}
        queue: deque[Cell] = deque([self.entry])
        while queue:
            cell = queue.popleft()
            if cell == self.exit:
                break
            for nxt in self.neighbours(cell):
                if nxt not in previous:
                    previous[nxt] = cell
                    queue.append(nxt)
        if self.exit not in previous:
            return []
        path: List[Cell] = []
        step: Optional[Cell] = self.exit
        while step is not None:
            path.append(step)
            step = previous[step]
        path.reverse()
        return path

    def solution_directions(self) -> str:
        """Return the shortest path as letters ``N``, ``E``, ``S``, ``W``."""
        path = self.solution()
        letters: List[str] = []
        for (x1, y1), (x2, y2) in zip(path, path[1:]):
            for direction, (dx, dy) in STEP.items():
                if (x1 + dx, y1 + dy) == (x2, y2):
                    letters.append(LETTER[direction])
        return "".join(letters)

    def neighbours(self, cell: Cell) -> List[Cell]:
        """Return the cells reachable from *cell* through an open wall."""
        x, y = cell
        result: List[Cell] = []
        for direction in DIRECTIONS:
            if not self.grid[y][x] & direction:
                dx, dy = STEP[direction]
                result.append((x + dx, y + dy))
        return result

    def is_open(self, cell: Cell, direction: int) -> bool:
        """Tell whether the wall of *cell* on *direction* is open."""
        x, y = cell
        return not self.grid[y][x] & direction

    def to_hex(self) -> List[str]:
        """Return the grid as rows of hexadecimal digits."""
        self._require_grid()
        return ["".join(f"{walls:X}" for walls in row) for row in self.grid]

    # ------------------------------------------------------------------ #
    # Validation
    # ------------------------------------------------------------------ #
    def _inside(self, cell: Cell) -> bool:
        x, y = cell
        return 0 <= x < self.width and 0 <= y < self.height

    def _check_endpoints(self) -> None:
        for name, cell in (("entry", self.entry), ("exit", self.exit)):
            if not self._inside(cell):
                raise MazeError(
                    f"{name} {cell[0]},{cell[1]} is outside the maze "
                    f"(0..{self.width - 1}, 0..{self.height - 1})"
                )
        if self.entry == self.exit:
            raise MazeError("entry and exit must be different cells")

    def _require_grid(self) -> None:
        if not self.grid:
            raise MazeError("call generate() first")

    # ------------------------------------------------------------------ #
    # "42" pattern
    # ------------------------------------------------------------------ #
    def _place_pattern(self) -> None:
        """Close the "42" cells, centred, keeping a free border around."""
        self.pattern_cells = set()
        self.pattern_omitted = False
        if not self.want_pattern:
            return
        if (self.width < PATTERN_WIDTH + 2
                or self.height < PATTERN_HEIGHT + 2):
            self.pattern_omitted = True
            return
        ox = (self.width - PATTERN_WIDTH) // 2
        oy = (self.height - PATTERN_HEIGHT) // 2
        cells = {
            (ox + col, oy + row)
            for row, line in enumerate(PATTERN_42)
            for col, char in enumerate(line)
            if char == "#"
        }
        for name, cell in (("entry", self.entry), ("exit", self.exit)):
            if cell in cells:
                raise MazeError(
                    f"{name} {cell[0]},{cell[1]} falls inside the '42' "
                    f"pattern"
                )
        self.pattern_cells = cells

    # ------------------------------------------------------------------ #
    # Wall helpers
    # ------------------------------------------------------------------ #
    def _free(self, cell: Cell) -> bool:
        """True for a cell inside the maze that is not part of the "42"."""
        return self._inside(cell) and cell not in self.pattern_cells

    def _step(self, cell: Cell, direction: int) -> Cell:
        dx, dy = STEP[direction]
        return cell[0] + dx, cell[1] + dy

    def _open(self, cell: Cell, direction: int) -> None:
        """Open a wall on both sides, so the encoding stays coherent."""
        other = self._step(cell, direction)
        self.grid[cell[1]][cell[0]] &= ~direction
        self.grid[other[1]][other[0]] &= ~OPPOSITE[direction]

    def _close(self, cell: Cell, direction: int) -> None:
        other = self._step(cell, direction)
        self.grid[cell[1]][cell[0]] |= direction
        self.grid[other[1]][other[0]] |= OPPOSITE[direction]

    def _free_cells(self) -> List[Cell]:
        return [
            (x, y)
            for y in range(self.height)
            for x in range(self.width)
            if (x, y) not in self.pattern_cells
        ]

    # ------------------------------------------------------------------ #
    # Perfect maze algorithms
    # ------------------------------------------------------------------ #
    def _carve_backtracker(self) -> None:
        """Recursive backtracker (randomized depth-first search).

        Implemented with an explicit stack instead of recursion, so big
        mazes never hit Python's recursion limit.
        """
        start = self.entry
        visited: Set[Cell] = {start}
        stack: List[Cell] = [start]
        while stack:
            cell = stack[-1]
            options = [
                d for d in DIRECTIONS
                if self._free(self._step(cell, d))
                and self._step(cell, d) not in visited
            ]
            if not options:
                stack.pop()
                continue
            direction = self._rng.choice(options)
            nxt = self._step(cell, direction)
            self._open(cell, direction)
            visited.add(nxt)
            stack.append(nxt)

    def _carve_prim(self) -> None:
        """Randomized Prim: grow the maze from a random frontier wall."""
        start = self.entry
        visited: Set[Cell] = {start}
        frontier: List[Tuple[Cell, int]] = [(start, d) for d in DIRECTIONS]
        while frontier:
            index = self._rng.randrange(len(frontier))
            frontier[index], frontier[-1] = frontier[-1], frontier[index]
            cell, direction = frontier.pop()
            nxt = self._step(cell, direction)
            if not self._free(nxt) or nxt in visited:
                continue
            self._open(cell, direction)
            visited.add(nxt)
            frontier.extend((nxt, d) for d in DIRECTIONS)

    # ------------------------------------------------------------------ #
    # Pac-Man board (PERFECT=False)
    # ------------------------------------------------------------------ #
    def _degree(self, cell: Cell) -> int:
        return len(self.neighbours(cell))

    def _creates_open_area(self, cell: Cell, direction: int) -> bool:
        """True if an open 3x3 block contains the wall just opened."""
        other = self._step(cell, direction)
        min_x, max_x = sorted((cell[0], other[0]))
        min_y, max_y = sorted((cell[1], other[1]))
        for x0 in range(max_x - 2, min_x + 1):
            for y0 in range(max_y - 2, min_y + 1):
                if self._block_open(x0, y0):
                    return True
        return False

    def _block_open(self, x0: int, y0: int) -> bool:
        """True if the 3x3 block with top-left ``(x0, y0)`` is all open."""
        if not (self._inside((x0, y0)) and self._inside((x0 + 2, y0 + 2))):
            return False
        for y in range(y0, y0 + 3):
            for x in range(x0, x0 + 3):
                if x < x0 + 2 and not self.is_open((x, y), EAST):
                    return False
                if y < y0 + 2 and not self.is_open((x, y), SOUTH):
                    return False
        return True

    def _try_open(self, cell: Cell, direction: int) -> bool:
        """Open a wall unless it would make a too wide open area."""
        self._open(cell, direction)
        if self._creates_open_area(cell, direction):
            self._close(cell, direction)
            return False
        return True

    def _closed_inner_walls(self, cell: Cell) -> List[int]:
        """Closed walls of *cell* that lead to another free cell."""
        return [
            d for d in DIRECTIONS
            if not self.is_open(cell, d) and self._free(self._step(cell, d))
        ]

    def _braid(self) -> None:
        """Remove dead-ends by opening one extra wall in each of them.

        Opening towards another dead-end is preferred: it fixes two
        dead-ends with a single wall.
        """
        dead_ends = [c for c in self._free_cells() if self._degree(c) == 1]
        self._rng.shuffle(dead_ends)
        for cell in dead_ends:
            if self._degree(cell) != 1:
                continue
            options = self._closed_inner_walls(cell)
            self._rng.shuffle(options)
            options.sort(
                key=lambda d: self._degree(self._step(cell, d)) != 1
            )
            for direction in options:
                if self._try_open(cell, direction):
                    break

    def _loops(self) -> int:
        """Independent cycles of the graph: edges - nodes + 1."""
        cells = self._free_cells()
        edges = sum(self._degree(c) for c in cells) // 2
        return edges - len(cells) + 1

    def _add_loops(self, wanted: int) -> None:
        """Open random walls until the board has *wanted* loops."""
        candidates = [
            (cell, d)
            for cell in self._free_cells()
            for d in (EAST, SOUTH)
            if self._free(self._step(cell, d))
        ]
        self._rng.shuffle(candidates)
        for cell, direction in candidates:
            if self._loops() >= wanted:
                return
            if not self.is_open(cell, direction):
                self._try_open(cell, direction)
