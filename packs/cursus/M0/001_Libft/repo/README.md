*This project has been created as part of the 42 curriculum by xalara.*

# Libft

## Description

Libft is the first project of the 42 common core: a personal C library that
recreates a set of functions from the C standard library and adds extra
utilities (string helpers, file descriptor output and a singly linked list)
that will be reused in later projects.

Writing them from scratch is a way to really understand:

- pointers and pointer arithmetic
- strings and raw memory (`char *` vs `unsigned char *`, overlapping areas)
- dynamic memory: checking every `malloc` and freeing on error paths
- linked lists
- headers, object files, static libraries and Makefiles
- edge cases: empty strings, `n == 0`, integer limits, failed allocations

The result is a static library, `libft.a`.

### Library contents

**Characters**

| Function | Description |
|---|---|
| `ft_isalpha` | checks for an alphabetic character |
| `ft_isdigit` | checks for a decimal digit |
| `ft_isalnum` | checks for an alphanumeric character |
| `ft_isascii` | checks for a 7-bit ASCII value |
| `ft_isprint` | checks for a printable character |
| `ft_toupper` | converts a lowercase letter to uppercase |
| `ft_tolower` | converts an uppercase letter to lowercase |

**Strings**

| Function | Description |
|---|---|
| `ft_strlen` | length of a string |
| `ft_strchr` | first occurrence of a character |
| `ft_strrchr` | last occurrence of a character |
| `ft_strncmp` | compares at most `n` characters |
| `ft_strlcpy` | size-bounded copy, always NUL-terminated |
| `ft_strlcat` | size-bounded concatenation |
| `ft_strnstr` | finds a substring within the first `len` characters |
| `ft_strdup` | allocates a copy of a string |
| `ft_atoi` | converts a string to an `int` |

**Memory**

| Function | Description |
|---|---|
| `ft_memset` | fills memory with a byte |
| `ft_bzero` | fills memory with zeros |
| `ft_memcpy` | copies non-overlapping memory |
| `ft_memmove` | copies memory, handling overlap |
| `ft_memchr` | finds a byte in memory |
| `ft_memcmp` | compares two memory areas |
| `ft_calloc` | allocates zeroed memory, with overflow check |

**Additional functions**

| Function | Description |
|---|---|
| `ft_substr` | allocates a substring of `s` |
| `ft_strjoin` | allocates the concatenation of two strings |
| `ft_strtrim` | removes a set of characters from both ends |
| `ft_split` | splits a string by a delimiter into a NULL-terminated array |
| `ft_itoa` | converts an `int` to an allocated string |
| `ft_strmapi` | builds a new string applying `f(i, c)` to each character |
| `ft_striteri` | applies `f(i, &c)` to each character in place |
| `ft_putchar_fd` | writes a character to a file descriptor |
| `ft_putstr_fd` | writes a string to a file descriptor |
| `ft_putendl_fd` | writes a string followed by a newline |
| `ft_putnbr_fd` | writes an integer to a file descriptor |

**Linked list** (`t_list`: `void *content`, `t_list *next`)

| Function | Description |
|---|---|
| `ft_lstnew` | creates a new node |
| `ft_lstadd_front` | adds a node at the beginning |
| `ft_lstadd_back` | adds a node at the end |
| `ft_lstsize` | counts the nodes |
| `ft_lstlast` | returns the last node |
| `ft_lstdelone` | frees one node and its content |
| `ft_lstclear` | frees a whole list |
| `ft_lstiter` | applies a function to every content |
| `ft_lstmap` | builds a new list applying a function to every content |

## Instructions

### Compilation

```sh
make        # builds libft.a
make clean  # removes the object files
make fclean # removes the object files and libft.a
make re     # fclean + all
```

Everything is compiled with `cc -Wall -Wextra -Werror` and archived with
`ar rcs`. The list functions are part of the library itself, so a plain
`make` builds all of them.

### Using the library

Include the header and link against `libft.a`:

```c
#include "libft.h"
#include <stdio.h>

int	main(void)
{
	char	**words;
	int		i;

	words = ft_split("hello from libft", ' ');
	if (!words)
		return (1);
	i = 0;
	while (words[i])
	{
		printf("%s\n", words[i]);
		free(words[i]);
		i++;
	}
	free(words);
	return (0);
}
```

```sh
cc main.c -I. -L. -lft -o demo
./demo
```

- `-I.` where to find `libft.h`
- `-L.` where to find `libft.a`
- `-lft` link with `libft.a`

## Technical choices

- One function per `.c` file, with a single public header, `libft.h`.
  Helpers are `static`, so the only global symbols are the `ft_` functions.
- Every function that allocates memory returns `NULL` if `malloc` fails and
  frees whatever it had already allocated (for example, the words already
  copied in `ft_split`, or the nodes already created in `ft_lstmap`).
- Memory functions check the length before reading a byte, so they never read
  past `n`, not even one byte.
- `ft_calloc` checks `nmemb * size` for overflow before calling `malloc`.

## Resources

- `man` pages for the original functions (`man 3 strlcpy`, `man 3 memmove`,
  `man 3 calloc`, ...).
- The Libft subject.
- [libftTester](https://github.com/Tripouille/libftTester) by Tripouille, run
  under `valgrind`, plus my own test runner, which also makes `malloc` fail on
  purpose and puts buffers right before a protected memory page to catch
  out-of-bounds reads.
- `norminette` for the 42 coding standard.

### AI usage

I used AI tools (Claude) as a reviewer and a tutor, not to write the library.
I used them to:

- explain why a test crashed or leaked (for example, reading `p[n]` after the
  loop in `ft_memchr`, or a double free on the error path of `ft_lstmap`);
- point out edge cases I had not thought of (empty strings, `n == 0`, failed
  allocations);
- explain norminette errors.

I wrote and fixed the code myself, and tested every function.
