*This project has been created as part of the 42 curriculum by xalara, gabgalla.*

# push_swap

## Description

push_swap sorts a list of integers using two stacks, `a` and `b`, and a
fixed set of 11 operations. The program does not print the sorted numbers:
it prints the shortest list of operations it can find that leaves `a`
sorted in ascending order. The fewer operations, the better the grade.

| Operation | Effect |
|---|---|
| `sa` / `sb` / `ss` | swap the first two elements of `a` / `b` / both |
| `pa` / `pb` | take the top of `b` and put it on `a` / the other way round |
| `ra` / `rb` / `rr` | rotate up: the first element becomes the last |
| `rra` / `rrb` / `rrr` | rotate down: the last element becomes the first |

This version (42next) also asks for:

- a **disorder index** of the input, measured before any move:
  `mistakes / pairs`, where `pairs` are all the pairs `(i, j)` with `i`
  before `j`, and `mistakes` the pairs that are in the wrong order;
- **four strategies**, one per complexity class, that can be forced with a
  flag: `--simple` (O(n²)), `--medium` (O(n√n)), `--complex` (O(n log n))
  and `--adaptive` (the default), which picks a method from the disorder;
- an optional `--bench` mode that prints statistics to `stderr`.

## Instructions

### Compilation

```sh
make        # builds libft and push_swap
make clean  # removes the object files
make fclean # also removes push_swap and libft.a
make re     # fclean + all
```

### Usage

```sh
./push_swap 2 1 3 6 5 8              # one number per argument
./push_swap "2 1 3 6 5 8"            # or several in one argument
./push_swap --complex 4 67 3 87 23   # force a strategy
./push_swap --bench 4 67 3 87 23     # statistics on stderr

ARG="4 67 3 87 23"; ./push_swap $ARG | ./checker_linux $ARG   # OK
ARG="4 67 3 87 23"; ./push_swap $ARG | wc -l                 # operations
```

- Invalid input (not a number, out of `int` range, duplicates, two strategy
  flags) prints `Error` to `stderr`.
- With no arguments, or an already sorted list, nothing is printed.

`--bench` only writes to `stderr`, so `stdout` stays the same and the
checker still works:

```
[bench] disorder:  49.72%
[bench] strategy:  Adaptive / O(n√n)
[bench] total_ops:  572
[bench] sa:  0  sb:  0  ss:  0  pa:  100  pb:  100
[bench] ra:  ...
```

## Algorithms

### Common ground

- **Normalization.** Right after parsing, every number is replaced by its
  rank (`42 -7 100 5` → `2 0 3 1`). The order is the same, so sorting the
  ranks sorts the numbers, but now the values are `0..n-1`: radix can read
  their bits, chunks know that "the k smallest" are `0..k-1`, and "the
  smallest left in `a`" is just the next rank.
- **Shortest rotation.** To bring the element at position `p` of a stack of
  size `s` to the top, the program uses `ra` `p` times if it is in the top
  half, and `rra` `s - p` times otherwise (`rotate_to_top_a/b` in
  `stack_utils.c`). That costs at most `s / 2`.

### `--simple`: selection sort, O(n²)

Bring the smallest element left in `a` to the top (shortest rotation), `pb`
it, and repeat. As soon as what is left in `a` is already sorted, stop.
Then `pa` everything back: `b` holds the smallest ones with the largest on
top, so they land in order.

Each step costs at most `n / 2` rotations plus one push, and there are up
to `n` steps: **O(n²)** operations (about `n² / 4`).

### `--medium`: chunks ("butterfly"), O(n√n)

1. Push everything to `b` in ranges of about `1.5 √n` ranks. With `i`
   elements already in `b`, the top of `a` is accepted if its rank is
   `<= i + range`; if it is also `<= i`, it goes to the bottom of `b`
   (`pb` + `rb`), otherwise it stays on top (`pb`). If it is not in range,
   `ra`. The smallest rank left in `a` is always `<= i`, so there is always
   something to push.
2. `b` ends up shaped like a butterfly: large values near both ends, small
   ones in the middle. Take the largest of `b` (shortest rotation) and `pa`
   it, until `b` is empty.

An element waits at most one range of rotations before it is pushed, and
the largest of `b` is always close to one end: **O(n√n)** operations.

### `--complex`: binary LSD radix sort, O(n log n)

For each bit of the ranks, from the least significant one: go through all
of `a`, `pb` the elements with that bit at 0 and `ra` the ones with 1, then
`pa` everything back. It is stable, so after the last bit `a` is sorted.

There are `⌈log₂ n⌉` passes and each one costs at most `2n` operations:
**O(n log n)**, and the same count for any input of a given size (1084 for
100 numbers, 6784 for 500).

### `--adaptive` (default)

The disorder `d` is measured before moving anything, and then:

| Disorder | Method | Class |
|---|---|---|
| `d < 0.2` | LIS + insertion (below) | O(n) |
| `0.2 <= d < 0.5` | chunks (`--medium`) | O(n√n) |
| `d >= 0.5` | radix (`--complex`) | O(n log n) |

**Low disorder: LIS + insertion.** If the list is almost sorted, most of it
is already in the right order. The longest increasing subsequence (LIS) of
`a` is exactly the largest part that can stay where it is:

1. Compute the LIS on a copy of `a` (this is computation, not operations).
2. Go through `a`: `ra` the elements of the LIS, `pb` the others. Stop as
   soon as the last one outside the LIS has been pushed.
3. Put each element of `b` back in its gap in `a` (above the smallest
   value that is greater than it), with the shortest rotation.
4. Rotate `0` to the top.

Step 2 costs at most `n`. With `k` elements outside the LIS, step 3 costs
at most `k · n / 2`, and with little disorder `k` is small and the elements
go back close to where they were taken from: **O(n + k·n)**, O(n) when `k`
is bounded. For 500 numbers with 25 swaps between neighbours it takes
about 1050 operations.

**Why these thresholds.** The values come from the subject, and they match
how the methods behave:

- Below 0.2 most of the list is already in place, so a method that keeps
  that part and only moves the rest beats any method that moves
  everything (radix always does `⌈log₂ n⌉` full passes).
- A random list has a disorder of about 0.5. Above that, the input looks
  random or reversed, and radix gives a guaranteed `2n ⌈log₂ n⌉` bound.
- In between, the input is partly ordered and chunks adapt better than
  radix.

### Space

All strategies use the two stacks, O(n) nodes in total. `normalize` and the
LIS use temporary `int` arrays of size `n` (and `2n`), so the extra space is
**O(n)** in every case.

### Measured operations

Averages over 10 random inputs:

| Strategy | 100 numbers | 500 numbers |
|---|---|---|
| `--simple` | ~1460 | ~32500 |
| `--medium` | ~570 | ~5060 |
| `--complex` | 1084 | 6784 |
| default | ~930 | ~5930 |

## Contributions

| Login | Parts |
|---|---|
| xalara | parsing, normalization, disorder index, `--bench`, sorting strategies, README |
| gabgalla | stack operations (`push.c`) |

## Resources

- The push_swap subject (42next version) and its `checker_linux`.
- Big-O notation and sorting algorithms: selection sort, bucket/chunk sort,
  radix sort (LSD), and the longest increasing subsequence
  ([Wikipedia: Longest increasing subsequence](https://en.wikipedia.org/wiki/Longest_increasing_subsequence)).
- `norminette` for the 42 coding standard and `valgrind` for leaks.

### AI usage

We used AI tools (Claude) to:

- download and read the right version of the subject;
- write a black-box test runner (errors, every strategy against the
  checker, performance limits, `--bench`, leaks);
- write the sorting strategies (`sort_simple.c`, `sort_medium.c`,
  `sort_complex.c`, `sort_low.c`) and the helpers in `stack_utils.c`, and
  explain them line by line;
- review norminette errors and this README.

We reviewed and tested all of it and can explain every part of the code.
