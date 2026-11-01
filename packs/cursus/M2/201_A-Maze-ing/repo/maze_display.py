"""Terminal (ASCII + ANSI colours) rendering of a maze."""

from __future__ import annotations

from dataclasses import dataclass
from typing import List, Set, Tuple

from mazegen import EAST, SOUTH, Cell, MazeGenerator

RESET = "\033[0m"
BLOCK = "  "

# (name, ANSI 256 colour code) used as background colours.
WALL_COLOURS: Tuple[Tuple[str, int], ...] = (
    ("white", 255),
    ("blue", 27),
    ("green", 34),
    ("magenta", 127),
    ("orange", 208),
    ("grey", 244),
)
PATTERN_COLOURS: Tuple[Tuple[str, int], ...] = (
    ("yellow", 220),
    ("red", 196),
    ("cyan", 51),
    ("pink", 213),
    ("same as walls", -1),
)
PATH_COLOUR = 39
ENTRY_COLOUR = 46
EXIT_COLOUR = 160
OPEN = -1


@dataclass
class DisplayState:
    """What the user can toggle in the interactive view."""

    show_path: bool = False
    wall_index: int = 0
    pattern_index: int = 0

    @property
    def wall_colour(self) -> int:
        """ANSI code of the current wall colour."""
        return WALL_COLOURS[self.wall_index][1]

    @property
    def pattern_colour(self) -> int:
        """ANSI code of the current "42" colour."""
        colour = PATTERN_COLOURS[self.pattern_index][1]
        return self.wall_colour if colour < 0 else colour


def _paint(colour: int) -> str:
    if colour == OPEN:
        return BLOCK
    return f"\033[48;5;{colour}m{BLOCK}{RESET}"


def render(gen: MazeGenerator, state: DisplayState) -> str:
    """Return the coloured drawing of the maze as a string.

    The drawing is a grid of ``(2 * height + 1) x (2 * width + 1)`` blocks:
    odd/odd blocks are cells, the others are walls or wall corners.
    """
    path = gen.solution() if state.show_path else []
    path_cells: Set[Cell] = set(path)
    path_links: Set[Tuple[Cell, Cell]] = set()
    for a, b in zip(path, path[1:]):
        path_links.add((min(a, b), max(a, b)))

    lines: List[str] = []
    for by in range(2 * gen.height + 1):
        row: List[str] = []
        for bx in range(2 * gen.width + 1):
            row.append(_paint(_block_colour(gen, state, bx, by,
                                            path_cells, path_links)))
        lines.append("".join(row))
    return "\n".join(lines)


def _block_colour(gen: MazeGenerator, state: DisplayState, bx: int,
                  by: int, path_cells: Set[Cell],
                  path_links: Set[Tuple[Cell, Cell]]) -> int:
    """Pick the colour of one block (OPEN = terminal background)."""
    x, y = bx // 2, by // 2
    if bx % 2 and by % 2:                       # a cell
        cell = (x, y)
        if cell in gen.pattern_cells:
            return state.pattern_colour
        if cell == gen.entry:
            return ENTRY_COLOUR
        if cell == gen.exit:
            return EXIT_COLOUR
        return PATH_COLOUR if cell in path_cells else OPEN
    if bx % 2:                                  # horizontal wall
        above, below = (x, y - 1), (x, y)
        if 0 < by < 2 * gen.height and gen.is_open(above, SOUTH):
            return PATH_COLOUR if (above, below) in path_links else OPEN
        return _wall_colour(gen, state, [above, below])
    if by % 2:                                  # vertical wall
        left, right = (x - 1, y), (x, y)
        if 0 < bx < 2 * gen.width and gen.is_open(left, EAST):
            return PATH_COLOUR if (left, right) in path_links else OPEN
        return _wall_colour(gen, state, [left, right])
    corners = [(x - 1, y - 1), (x, y - 1), (x - 1, y), (x, y)]
    return _wall_colour(gen, state, corners)


def _wall_colour(gen: MazeGenerator, state: DisplayState,
                 cells: List[Cell]) -> int:
    """Walls between "42" cells take the pattern colour."""
    inside = [c for c in cells
              if 0 <= c[0] < gen.width and 0 <= c[1] < gen.height]
    if inside and all(c in gen.pattern_cells for c in inside) \
            and len(inside) == len(cells):
        return state.pattern_colour
    return state.wall_colour


def legend(gen: MazeGenerator, state: DisplayState) -> str:
    """Return the text shown under the maze."""
    mode = "perfect" if gen.perfect else "Pac-Man board"
    lines = [
        f"{_paint(ENTRY_COLOUR)} entry {gen.entry[0]},{gen.entry[1]}   "
        f"{_paint(EXIT_COLOUR)} exit {gen.exit[0]},{gen.exit[1]}   "
        f"{_paint(PATH_COLOUR)} path "
        f"({'shown' if state.show_path else 'hidden'})",
        f"{gen.width}x{gen.height}, {mode}, {gen.algorithm}, "
        f"seed {gen.seed}",
    ]
    return "\n".join(lines)
