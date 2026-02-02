# TP 1 — C refresher

## Objectives

- warm up dynamic allocation, arrays and command-line parsing in C;
- implement a few classic algorithms with careful edge-case handling;
- compile with `-Wall -Wextra -g` and keep the programs warning-free.

The session contains four independent programs.

---

## 1. Prime factorisation (`facteurs`)

`smallest_divisor(n)` returns the smallest divisor of `n` greater than one,
or `n` when `n` is prime.  Even numbers are handled immediately, then only odd
candidates are tested.  The loop is written

```c
for (divisor = 3; divisor <= number / divisor; divisor += 2)
```

rather than `divisor * divisor <= number`: the division form cannot overflow
when `number` is close to `LLONG_MAX`.

The factorisation repeatedly extracts the smallest divisor.  Because the
divisor is always prime, the output is the prime factorisation in
non-decreasing order.  For the statement example:

```
1170701532 = 2 x 2 x 3 x 3 x 3 x 7 x 7 x 7 x 11 x 13 x 13 x 17
```

The worst case is a prime number, with `O(sqrt(n))` trial divisions.  The
memory usage is `O(1)`.

`0` and `1` have no prime factor and are printed as themselves; a negative
number starts with the factor `-1`.

---

## 2. Look-and-say (`enigme`)

A line is stored in a growable `Sequence`.  One step scans the current line,
detects maximal runs of equal values and writes `(run length, value)` for each
run.  For example `1 2 1 1` becomes `1 1 1 2 2 1`, then `3 1 2 2 1 1`.

The number of lines and the initial line are command-line arguments:

```bash
./bin/enigme 3 7
./bin/enigme "1 2 1 1" 3
```

Each line is built in `O(length)` and the lengths grow exponentially in the
worst case, so the total cost is proportional to the total output size.

---

## 3. Intersection of two lines (`intersection`)

For two points `P1(x1, y1)` and `P2(x2, y2)`, the slope is

```
a = (y2 - y1) / (x2 - x1)
b = y1 - a * x1
```

The program deliberately separates the vertical case (`x1 == x2`), where the
slope is undefined, instead of dividing by zero.

| Case | Result |
|---|---|
| both lines vertical, different `x` | parallel, no intersection |
| both lines vertical, same `x` | confounded, infinitely many intersections |
| one vertical | substitute `x` into the other equation |
| same slope, same intercept | confounded |
| same slope, different intercept | parallel |
| otherwise | `x = (b2 - b1) / (a1 - a2)`, then `y = a1 x + b1` |

Floating-point comparisons use a relative epsilon (`1e-9`), so almost-parallel
lines are not mistaken for parallel ones because of rounding.

The statement works with **segments** even though it derives line equations.
The program therefore also checks whether the intersection lies inside both
segments and prints whether it does.

```bash
./bin/intersection 0 0 4 4 0 4 4 0
# Intersection des droites: (2.000000, 2.000000)
# Le point appartient aux deux segments
```

---

## 4. Insertion sort (`tri_insertion`)

The invariant before iteration `i` is: `values[0..i-1]` is sorted.  The key
`values[i]` is saved, every larger element of the prefix is shifted one slot
to the right, and the key is inserted in the hole.

```c
for (index = 1; index < length; index++) {
    int key = values[index];
    long position = (long)index - 1;
    while (position >= 0 && values[position] > key) {
        values[position + 1] = values[position];
        position--;
    }
    values[position + 1] = key;
}
```

- best case (already sorted): `n - 1` comparisons, `Theta(n)`;
- worst case (reverse sorted): `n(n-1)/2` comparisons and shifts,
  `Theta(n^2)`;
- in place, `O(1)` extra memory.

`--verbose` prints the array after each insertion, which makes the algorithm
easy to follow on the blackboard example.

---

## Build, run and test

```bash
make -C tp01
./tp01/bin/facteurs 1170701532
./tp01/bin/enigme 3 7
./tp01/bin/intersection 0 0 4 4 0 4 4 0
./tp01/bin/tri_insertion -v 5 2 4 6 1 3
python3 -m pytest tests/test_tp01.py -q
```

The tests cover the statement example, prime inputs, `0`, `1`, negative
numbers, the exact look-and-say lines, parallel/confounded/vertical lines,
intersections outside the segments, duplicates, negative values and the
verbose insertion trace.
