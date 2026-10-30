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
| `ft_print_chars.c` | `%c`, `%s` and `%%`, plus `ft_print_free`, which prints and frees an allocated string |
| `ft_print_nbrs.c` | `%d`, `%i` and `%u` |
| `ft_print_hex.c` | `%x`, `%X` and `%p` |
| `ft_printf.h` | prototypes |
| `libft/` | my libft, built and included in the library; it adds `ft_itoa_base` to the Libft functions |

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

Characters and strings are written directly with `write`. A `NULL` string
prints `(null)`.

Numbers are first converted to an allocated string with my libft, and then
printed:

- `%d` and `%i` use `ft_itoa`, which already handles the sign and `INT_MIN`.
- `%u`, `%x` and `%X` use `ft_itoa_base(n, base, b, 0)`. It takes an
  `unsigned long` and the symbols of the base (`"0123456789"`,
  `"0123456789abcdef"` or `"0123456789ABCDEF"`). It is recursive: each call
  goes one digit deeper, the deepest call already knows how many digits there
  are and allocates the string once, and every call writes its digit on the
  way back.
- `%p` writes `0x` and reuses the hexadecimal printer, which is why that
  printer takes an `unsigned long`: an address does not fit in an
  `unsigned int`. A `NULL` pointer prints `(nil)`, like glibc.

Every number goes through `ft_print_free`, which returns `-1` if the string is
`NULL` (a failed `malloc`), prints it and frees it. That is the only place
that frees memory, so a failed allocation or a failed `write` never leaks.

## Resources

- `man 3 printf` and `man 3 stdarg`.
- The ft_printf subject.
- My own test runner, which compares every case with the real `printf`
  (`snprintf` from glibc) and checks the `make` rules.
- `norminette` for the 42 coding standard.

### AI usage

I used AI tools (Claude) as a tutor and, for part of the code, as a pair
programmer:

- it set up the starting skeleton: `ft_printf` and `ft_conv`, the split into
  files and the rule that every printer returns what it wrote or `-1`, and it
  explained how variadic arguments work and why `%c` is read as an `int`;
- I wrote `ft_print_char` and `ft_print_str`;
- `ft_itoa_base` is my function from the piscine (C07). Claude adapted it to
  `unsigned long`, without the sign, and moved it into my libft;
- after we agreed on the design (reuse `ft_itoa` and `ft_itoa_base` and free in
  one helper), Claude wrote `ft_print_free`, the number printers,
  `ft_print_hex` and `ft_print_ptr`;
- it pointed out edge cases (`NULL` strings and pointers, `INT_MIN`, failed
  allocations) and explained norminette errors.

I reviewed every function, can explain how each one works, and tested the
whole library with my test runner.
