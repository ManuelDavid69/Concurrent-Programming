# TP 2 — Binary heap and heap sort

`bin/tas` builds a min-heap by successive insertions, displays it in three
formats and extracts every element in increasing order.  `--sort` uses an
in-place max-heap sort on the same values.

```bash
make
./bin/tas
./bin/tas --display tree
./bin/tas --random 15 42
./bin/tas --sort 7 3 9 1 5
```

The heap is stored in an array: the left child of index `i` is `2i + 1`, the
right child is `2i + 2`, and the parent is `(i - 1) / 2`.  Insertion appends
the value and sifts it up; extraction replaces the root with the last value
and sifts it down.  Both operations are `O(log n)`, and building the heap by
insertions is `O(n log n)`.  Heap sort is in place and runs in `O(n log n)` in
all cases.
