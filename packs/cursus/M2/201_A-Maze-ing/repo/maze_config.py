"""Read and validate the A-Maze-ing configuration file."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Dict, Optional, Tuple

from mazegen import ALGORITHMS

MANDATORY_KEYS: Tuple[str, ...] = (
    "WIDTH", "HEIGHT", "ENTRY", "EXIT", "OUTPUT_FILE", "PERFECT",
)
OPTIONAL_KEYS: Tuple[str, ...] = ("SEED", "ALGORITHM")
MAX_SIDE = 200


class ConfigError(Exception):
    """Raised when the configuration file is missing or invalid."""


@dataclass
class MazeConfig:
    """All the options read from the configuration file."""

    width: int
    height: int
    entry: Tuple[int, int]
    exit: Tuple[int, int]
    output_file: str
    perfect: bool
    seed: Optional[int] = None
    algorithm: str = "backtracker"


def read_config(path: str) -> MazeConfig:
    """Parse the configuration file at *path*.

    Args:
        path: Path of a ``KEY=VALUE`` text file.

    Returns:
        The validated configuration.

    Raises:
        ConfigError: If the file cannot be read or is invalid.
    """
    try:
        with open(path, encoding="utf-8") as stream:
            lines = stream.readlines()
    except FileNotFoundError:
        raise ConfigError(f"configuration file not found: {path}")
    except IsADirectoryError:
        raise ConfigError(f"{path} is a directory, not a file")
    except (OSError, UnicodeDecodeError) as error:
        raise ConfigError(f"cannot read {path}: {error}")
    return build_config(parse_lines(lines))


def parse_lines(lines: list[str]) -> Dict[str, str]:
    """Turn the file lines into a ``{KEY: VALUE}`` dictionary.

    Empty lines and lines starting with ``#`` are ignored.

    Raises:
        ConfigError: On a malformed line, unknown key or duplicated key.
    """
    values: Dict[str, str] = {}
    for number, raw in enumerate(lines, start=1):
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        if "=" not in line:
            raise ConfigError(
                f"line {number}: expected KEY=VALUE, got '{line}'"
            )
        key, value = (part.strip() for part in line.split("=", 1))
        key = key.upper()
        if key not in MANDATORY_KEYS + OPTIONAL_KEYS:
            raise ConfigError(f"line {number}: unknown key '{key}'")
        if key in values:
            raise ConfigError(f"line {number}: key '{key}' is repeated")
        if not value:
            raise ConfigError(f"line {number}: '{key}' has no value")
        values[key] = value
    missing = [key for key in MANDATORY_KEYS if key not in values]
    if missing:
        raise ConfigError(f"missing mandatory key(s): {', '.join(missing)}")
    return values


def build_config(values: Dict[str, str]) -> MazeConfig:
    """Convert and check every value of the dictionary."""
    width = _parse_int("WIDTH", values["WIDTH"], 1, MAX_SIDE)
    height = _parse_int("HEIGHT", values["HEIGHT"], 1, MAX_SIDE)
    entry = _parse_cell("ENTRY", values["ENTRY"])
    exit_ = _parse_cell("EXIT", values["EXIT"])
    for name, (x, y) in (("ENTRY", entry), ("EXIT", exit_)):
        if not (0 <= x < width and 0 <= y < height):
            raise ConfigError(
                f"{name}={x},{y} is outside the maze "
                f"(x in 0..{width - 1}, y in 0..{height - 1})"
            )
    if entry == exit_:
        raise ConfigError("ENTRY and EXIT must be different cells")
    seed = None
    if "SEED" in values:
        seed = _parse_int("SEED", values["SEED"], 0, 2 ** 63)
    algorithm = values.get("ALGORITHM", "backtracker").lower()
    if algorithm not in ALGORITHMS:
        raise ConfigError(
            f"ALGORITHM must be one of: {', '.join(ALGORITHMS)}"
        )
    return MazeConfig(
        width=width,
        height=height,
        entry=entry,
        exit=exit_,
        output_file=values["OUTPUT_FILE"],
        perfect=_parse_bool("PERFECT", values["PERFECT"]),
        seed=seed,
        algorithm=algorithm,
    )


def _parse_int(key: str, text: str, low: int, high: int) -> int:
    try:
        number = int(text)
    except ValueError:
        raise ConfigError(f"{key} must be an integer, got '{text}'")
    if not low <= number <= high:
        raise ConfigError(f"{key} must be between {low} and {high}")
    return number


def _parse_cell(key: str, text: str) -> Tuple[int, int]:
    parts = text.split(",")
    if len(parts) != 2:
        raise ConfigError(f"{key} must look like x,y - got '{text}'")
    try:
        return int(parts[0]), int(parts[1])
    except ValueError:
        raise ConfigError(f"{key} coordinates must be integers: '{text}'")


def _parse_bool(key: str, text: str) -> bool:
    lowered = text.lower()
    if lowered in ("true", "1", "yes"):
        return True
    if lowered in ("false", "0", "no"):
        return False
    raise ConfigError(f"{key} must be True or False, got '{text}'")
