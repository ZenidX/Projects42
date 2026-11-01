"""A-Maze-ing: generate a maze from a configuration file and display it.

Usage:
    python3 a_maze_ing.py config.txt
"""

from __future__ import annotations

import random
import sys
from typing import List

from maze_config import ConfigError, MazeConfig, read_config
from maze_display import (
    PATTERN_COLOURS,
    WALL_COLOURS,
    DisplayState,
    legend,
    render,
)
from maze_output import write_maze
from mazegen import MazeError, MazeGenerator

MENU = """
=== A-Maze-ing ===
1. Re-generate a new maze
2. Show/Hide path from entry to exit
3. Change wall colour
4. Change '42' colour
5. Quit"""


def build_generator(config: MazeConfig) -> MazeGenerator:
    """Create and run the generator described by *config*."""
    gen = MazeGenerator(
        width=config.width,
        height=config.height,
        entry=config.entry,
        exit=config.exit,
        perfect=config.perfect,
        seed=config.seed,
        algorithm=config.algorithm,
    )
    generate(gen, config, None)
    return gen


def generate(gen: MazeGenerator, config: MazeConfig,
             seed: int | None) -> None:
    """Generate a maze, warn about the "42" and save the output file."""
    gen.generate(seed)
    if gen.pattern_omitted:
        print("Error: the maze is too small to draw the '42' pattern "
              "(it needs at least 9x7 cells); it was omitted.")
    write_maze(gen, config.output_file)


def show(gen: MazeGenerator, state: DisplayState) -> None:
    """Print the maze and its legend."""
    print(render(gen, state))
    print(legend(gen, state))


def interactive(gen: MazeGenerator, config: MazeConfig) -> None:
    """Show the maze and answer the menu until the user quits."""
    state = DisplayState()
    show(gen, state)
    while True:
        print(MENU)
        try:
            choice = input("Choice? (1-5): ").strip()
        except EOFError:
            print()
            return
        if choice == "1":
            generate(gen, config, random.randrange(2 ** 32))
            print(f"New maze saved to {config.output_file}")
        elif choice == "2":
            state.show_path = not state.show_path
        elif choice == "3":
            state.wall_index = (state.wall_index + 1) % len(WALL_COLOURS)
            print(f"Walls are now {WALL_COLOURS[state.wall_index][0]}")
        elif choice == "4":
            state.pattern_index = ((state.pattern_index + 1)
                                   % len(PATTERN_COLOURS))
            print(f"'42' is now {PATTERN_COLOURS[state.pattern_index][0]}")
        elif choice == "5":
            return
        else:
            print("Unknown choice, type a number between 1 and 5.")
            continue
        show(gen, state)


def main(argv: List[str]) -> int:
    """Run the program and return its exit status."""
    if len(argv) != 2:
        print(f"Usage: python3 {argv[0] if argv else 'a_maze_ing.py'} "
              f"config.txt")
        return 1
    try:
        config = read_config(argv[1])
        gen = build_generator(config)
    except ConfigError as error:
        print(f"Configuration error: {error}")
        return 1
    except MazeError as error:
        print(f"Impossible maze: {error}")
        return 1
    except OSError as error:
        print(f"Cannot write the output file: {error}")
        return 1
    print(f"Maze saved to {config.output_file}")
    try:
        interactive(gen, config)
    except OSError as error:
        print(f"Cannot write the output file: {error}")
        return 1
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main(sys.argv))
    except KeyboardInterrupt:
        print("\nBye!")
        sys.exit(130)
