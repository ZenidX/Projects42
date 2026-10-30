*This project has been created as part of the 42 curriculum by xalara.*

# get_next_line

## Description

`get_next_line` is a function that returns the next line read from a file
descriptor, one call at a time:

```c
char	*get_next_line(int fd);
```

Each call returns an allocated string with the next line, including the
terminating `\n` (unless the file ends without one), or `NULL` when there is
nothing left to read or an error occurs. The caller frees every line.

The point of the project is to learn:

- `static` variables, which keep their value between calls
- reading with `read()` in fixed-size chunks (`BUFFER_SIZE`) and gluing the
  pieces together
- careful memory management: no leaks, also when `malloc` or `read` fail
- keeping the state of several file descriptors at the same time (bonus)

### Files

| File | Contents |
|---|---|
| `get_next_line.c` | `get_next_line` and its reading logic |
| `get_next_line_utils.c` | string helpers (`gnl_strlen`, `gnl_strchr`, `gnl_strdup`, `gnl_strncpy`, `gnl_strncat`) |
| `get_next_line.h` | prototypes and default values |
| `*_bonus.c`, `*_bonus.h` | the bonus version |

## Instructions

There is no Makefile: the files are compiled together with the program that
uses them. `BUFFER_SIZE` sets how many bytes each `read()` asks for; it
defaults to 42 if it is not given.

```sh
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 \
	get_next_line.c get_next_line_utils.c main.c -o gnl
```

Example `main.c`:

```c
#include "get_next_line.h"
#include <fcntl.h>
#include <stdio.h>

int	main(void)
{
	int		fd;
	char	*line;

	fd = open("file.txt", O_RDONLY);
	if (fd < 0)
		return (1);
	line = get_next_line(fd);
	while (line)
	{
		printf("%s", line);
		free(line);
		line = get_next_line(fd);
	}
	close(fd);
	return (0);
}
```

For the bonus, compile the `_bonus` files instead and include
`get_next_line_bonus.h`.

## Algorithm

Each file descriptor has its own buffer of `BUFFER_SIZE + 1` bytes, kept in a
`static` array indexed by `fd` and allocated the first time that `fd` is used.
Between calls the buffer holds whatever was read after the last returned line.

On each call:

1. If the buffer already contains a `\n`, the line is cut out of it: the part
   up to and including the `\n` is returned and the rest is moved to the start
   of the buffer.
2. Otherwise, the buffer content becomes the start of the line and `read()` is
   called again and again, appending each chunk, until a chunk contains a
   `\n` (step 1 applies to it) or `read()` returns 0 (end of file).
3. If `read()` fails, or there is nothing to return, the line and the buffer
   of that `fd` are freed and the function returns `NULL`, so nothing stays
   allocated once a file has been read to the end.

The function only works on the bytes it needs, so it never reads the whole
file at once and works with any `BUFFER_SIZE`, from 1 to very large values.

### Bonus

The bonus asks for a single `static` variable and several file descriptors
read in turns without mixing their lines. Keeping one buffer per `fd` in a
single `static` array (`MAX_FD`, 1024 by default) already does both, so the
bonus files share the same code.

## Resources

- `man 2 read`, `man 2 open`, `man 3 malloc`.
- The get_next_line subject.
- Articles about `static` variables and file descriptors in C.
- My own test runner, compiled with several values of `BUFFER_SIZE`
  (1, 42, 10000000), which checks leaks and makes `malloc` fail on purpose.
- `norminette` for the 42 coding standard.

### AI usage

I used AI tools (Claude) as a reviewer and a tutor, not to write the
function. I used them to:

- explain why a test leaked or crashed, for example memory left in the
  buffer after the last line, or an error from `read()` that was not cleaned
  up;
- point out edge cases (`BUFFER_SIZE=1`, a file without a final `\n`, empty
  files, invalid file descriptors);
- explain norminette errors.

I wrote and fixed the code myself, and tested it.
