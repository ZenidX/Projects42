"""Write a generated maze to the output file."""

from __future__ import annotations

from mazegen import MazeGenerator


def format_maze(gen: MazeGenerator) -> str:
    """Return the full content of the output file.

    One hexadecimal digit per cell, row by row, then an empty line, the
    entry, the exit and the shortest path (``N``, ``E``, ``S``, ``W``).
    """
    lines = gen.to_hex()
    lines.append("")
    lines.append(f"{gen.entry[0]},{gen.entry[1]}")
    lines.append(f"{gen.exit[0]},{gen.exit[1]}")
    lines.append(gen.solution_directions())
    return "\n".join(lines) + "\n"


def write_maze(gen: MazeGenerator, path: str) -> None:
    """Write the maze of *gen* into *path*.

    Raises:
        OSError: If the file cannot be written.
    """
    with open(path, "w", encoding="utf-8") as stream:
        stream.write(format_maze(gen))
