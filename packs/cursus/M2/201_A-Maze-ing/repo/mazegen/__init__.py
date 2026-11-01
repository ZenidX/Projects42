"""mazegen - reusable maze generator from the A-Maze-ing project."""

from .generator import (
    ALGORITHMS,
    EAST,
    NORTH,
    SOUTH,
    WEST,
    Cell,
    MazeError,
    MazeGenerator,
)

__all__ = [
    "ALGORITHMS",
    "EAST",
    "NORTH",
    "SOUTH",
    "WEST",
    "Cell",
    "MazeError",
    "MazeGenerator",
]
__version__ = "1.0.0"
