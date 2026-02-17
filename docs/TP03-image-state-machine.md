# TP 3 — Image processing and state machines

## Objectives

- manipulate a binary image format (PPM) and its one-dimensional pixel array;
- implement a 3×3 convolution (mean blur) with a correct border policy;
- design and implement deterministic finite-state automata for a digicode,
  including a code with a repeated digit and a code validated by letters.

---

## 1. PPM and the pixel array

A PPM image is either ASCII (`P3`) or binary (`P6`).  The header contains the
magic number, the width, the height and the maximum channel value (255 here).
The pixel data is a sequence of RGB triplets.

The image is stored in a single `unsigned char *`:

```
offset(x, y, channel) = 3 * (y * width + x) + channel
```

For a 4×3 image, `(0,0) -> 0`, `(1,0) -> 3`, `(0,1) -> 12`.  A
one-dimensional array is much easier to pass to functions than a
`unsigned char **`, and it matches the memory layout of the file.

`image_load` accepts both P3 and P6, skips comments and whitespace, and checks
every dimension and pixel value.  `image_save` always writes P6.

---

## 2. Mean blur

The kernel is

```
1/9 * [ 1 1 1
        1 1 1
        1 1 1 ]
```

For each pixel and each channel, sum the nine values and divide by nine.  For
example, a pixel with neighbours summing to 1215 in red becomes `135`.

### Border policy: kernel crop

At the border, some neighbours do not exist.  Instead of inventing values, the
implementation ignores them and divides by the number of pixels actually
used:

- corner: 4 pixels;
- edge (not corner): 6 pixels;
- interior: 9 pixels.

This is the *kernel crop* strategy required by the statement.  The output has
the same dimensions as the input, and a uniform image is unchanged.  The
other strategies (crop the output, extend, mirror, wrap) are listed in the
statement; kernel crop is the simplest one that keeps every output pixel
defined.

The complexity is `O(width × height)` with a constant of at most nine
additions per channel per pixel.

---

## 3. Digicode

### Variant 1 — code 5742

The automaton has five states:

```
        5          7          4          2
 (0) -------> (1) ----> (2) ----> (3) ----> ((4)) porte ouverte
  ^            |         |         |
  |  autre     | autre   | autre   | autre
  +------------+---------+---------+
      5 depuis 1, 2 ou 3 ramène à l'état 1
```

A wrong digit other than `5` resets the automaton to state 0.  A `5` always
restarts a possible code, which is exactly the behaviour of the failure
function of the KMP automaton for the pattern `5742`.

### Variant 2 — code 5551

The repeated digit is the difficulty: after `55`, a new `5` must not reset the
automaton, it must keep the longest valid prefix.  The KMP failure function
gives:

| state | meaning | on `5` | on `1` | other |
|---|---|---|---|---|
| 0 | nothing | 1 | 0 | 0 |
| 1 | `5` | 2 | 0 | 0 |
| 2 | `55` | 3 | 0 | 0 |
| 3 | `555` | 3 | 4 (open) | 0 |

This is implemented generically by `automate_init`, so the same code handles
`5742`, `5551` and any other all-digit code.

### Variant 3 — code 8267B

Here `A` and `B` validate an attempt.  The code is only valid if the four
digits `8267` were typed since the previous validation letter:

- `8267B` → valid;
- `78267B` → invalid, because the initial `7` is part of the same attempt;
- `7A8267B` → first attempt `7A` is invalid, second attempt `8267B` is valid.

The implementation accumulates at most four digits.  A fifth digit marks the
attempt as too long, so it can no longer match.  `A` or `B` compares the
accumulated digits with `8267` and the letter with `B`, prints
`Code valide` or `Code invalide`, and starts a new attempt.

### Transition table

`bin/table` prints the KMP transition table for `5742` and `5551` and a
readable description of the `8267B` attempt automaton.  This is the written
answer to the "draw the state machine" part of the statement.

---

## Build, run and test

```bash
make -C tp03
./tp03/bin/flou tp03/sample.ppm /tmp/out.ppm
printf '1235742' | ./tp03/bin/digicode 5742
printf '125551'  | ./tp03/bin/digicode 5551
printf '7A8267B' | ./tp03/bin/digicode 8267B
./tp03/bin/table
python3 -m pytest tests/test_tp03.py -q
```

The tests use generated P3 and P6 images, check the kernel-crop values at a
corner, an interior pixel and a uniform image, and exercise every digicode
variant, including overlapping codes and the `A`/`B` validation rules.
