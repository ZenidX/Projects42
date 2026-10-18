*This project has been created as part of the 42 curriculum by xalara.*

# ft_printf

## Description

`ft_printf` is a recreation of the `printf` function from the C standard
library, packaged as a static library, `libftprintf.a`:

```c
int	ft_printf(const char *format, ...);
```

It writes `format` to the standard output, replacing each conversion with the
next argument, and returns the number of characters written, or `-1` if a
write fails.

The point of the project is to learn:

- variadic functions (`va_start`, `va_arg`, `va_end`)
- converting numbers to text in different bases
- building a library on top of another one (my `libft`)

### Supported conversions

| Conversion | Prints |
|---|---|
| `%c` | a single character |
| `%s` | a string (`(null)` for a `NULL` pointer) |
| `%p` | a pointer address in hexadecimal, with `0x` (`(nil)` for `NULL`) |
| `%d`, `%i` | a signed decimal integer |
| `%u` | an unsigned decimal integer |
| `%x` | an unsigned hexadecimal number, lowercase |
| `%X` | an unsigned hexadecimal number, uppercase |
| `%%` | a percent sign |

### Files

| File | Contents |
|---|---|
| `ft_printf.c` | `ft_printf`: walks the format string and dispatches each conversion |
| `ft_print_chars.c` | `%c`, `%s` and `%%` |
| `ft_print_nbrs.c` | `%d`, `%i` and `%u` |
| `ft_print_hex.c` | `%x`, `%X` and `%p` |
| `ft_printf.h` | prototypes |
| `libft/` | my libft, built and included in the library |

## Instructions

```sh
make        # builds libft and then libftprintf.a
make clean  # removes the object files (also libft's)
make fclean # removes the object files and the libraries
make re     # fclean + all
```

Everything is compiled with `cc -Wall -Wextra -Werror`. The Makefile builds
`libft/libft.a` first, copies it as `libftprintf.a` and adds the `ft_printf`
objects to it, so a program only needs to link with one library.

```c
#include "ft_printf.h"

int	main(void)
{
	int	n;

	n = ft_printf("%s has %d%% of %c, at %p\n", "libft", 100, 'C', &n);
	ft_printf("%d characters, %x in hex\n", n, n);
	return (0);
}
```

```sh
cc main.c -L. -lftprintf -o demo
./demo
```

## Algorithm

`ft_printf` reads the format string one character at a time. A normal
character is written as it is. A `%` is followed by a conversion letter:
`ft_conv` takes the next argument with `va_arg`, using the type that letter
expects, and calls the function that prints it. Every print function returns
how many characters it wrote, or `-1` if `write` fails; `ft_printf` adds them
up and stops at the first error.

Numbers are printed recursively from the most significant digit, using the
base string `"0123456789"` or `"0123456789abcdef"` (uppercase for `%X`). The
hexadecimal printer takes an `unsigned long` so that `%p` can reuse it after
writing `0x`. `%d` handles `INT_MIN`, which has no positive `int` equivalent.

The output is written directly with `write`, without a buffer, so there is
nothing to free or flush at the end.

## Resources

- `man 3 printf` and `man 3 stdarg`.
- The ft_printf subject.
- My own test runner, which compares every case with the real `printf`
  (`snprintf` from glibc) and checks the `make` rules.
- `norminette` for the 42 coding standard.

### AI usage

I used AI tools (Claude) as a reviewer and a tutor, not to write the library.
I used them to:

- explain how variadic arguments work and why `%c` is read as an `int`;
- point out edge cases (`NULL` strings and pointers, `INT_MIN`, a `%` at the
  end of the format string);
- explain norminette errors.

I wrote and fixed the code myself, and tested it.
