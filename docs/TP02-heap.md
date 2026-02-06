# TP 2 — Binary heap and heap sort

## Objectives

- implement a min-heap with an array (`entasser`, `detasser`);
- respect the two structural rules: heap order and a complete tree;
- display the heap in several readable formats;
- use the heap to sort (`tri par tas`);
- compare the array implementation with a dynamically allocated tree and an
  in-place heap sort.

---

## Representation

The heap is a complete binary tree stored in an array.  For a node at index
`i`:

```
parent      = (i - 1) / 2
left child  = 2 * i + 1
right child = 2 * i + 2
```

The array has a fixed capacity (the number of values to insert) and a current
size.  At the beginning `size = 0`; after each insertion it grows by one; each
extraction shrinks it by one.  Keeping `size` separate from `capacity` makes
the "full" and "empty" cases explicit.

Because the tree is complete, there are no holes: the last element is always
at `size - 1`, and the next insertion goes exactly there.  This is what makes
the array representation both compact and cache-friendly.

---

## Insertion (`entasser`)

1. write the new value at index `size` and increment `size`;
2. while the value is smaller than its parent, swap them;
3. stop at the root or as soon as the parent is smaller or equal.

The path from a leaf to the root has length `floor(log2 n)`, so the worst case
is `O(log n)`.  The number of swaps is the number of times the value moves
up, at most the height of the tree.

## Extraction (`detasser`)

1. save the root (the minimum);
2. move the last element to the root and decrement `size`;
3. sift it down: compare it with both children, swap with the smallest child
   while that child is smaller than the current value.

Again the cost is `O(log n)`.  Note that after moving the last element to the
root, only one subtree can violate the heap property at each step, so a single
downward path is enough.

The validity checker (`tas_est_valide`) tests every parent/child pair.  It is
used by the demo before displaying or extracting, and it is the property the
tests rely on for random inputs.

---

## Display

Three formats are available:

| Mode | Output |
|---|---|
| `line` | the underlying array on one line |
| `levels` | one line per depth level |
| `tree` | an indented ASCII tree with `\|--` and `\--` branches |

Example:

```
38
|-- 39
|   |-- 40
|   |   |-- 55
|   |   \-- 51
|   \-- 49
\-- 42
    |-- 62
    \-- 48
```

---

## Heap sort

The heap used for the demonstrations is a **min**-heap, which extracts values
in increasing order.  For an in-place sort, `tri_par_tas` builds a
**max**-heap and repeatedly swaps the root with the last element of the
current prefix.  The maximum goes to its final position, the prefix shrinks,
and the new root is sifted down.

```
build max-heap:      O(n)
n - 1 extractions:   O(n log n)
total:               O(n log n), in place, not stable
```

Building the heap bottom-up is linear: most nodes are near the leaves and
sift down very little.  This is better than `n` successive insertions, which
cost `O(n log n)`.

---

## Reflection

### Array versus dynamically allocated tree

| | Array | Linked nodes (`malloc`) |
|---|---|---|
| Memory per element | one `int` | one `int` + two pointers + allocator overhead |
| Locality | excellent | poor |
| Child access | arithmetic | pointer dereference |
| Complete-tree rule | automatic | must be maintained explicitly |
| Arbitrary shape | impossible | possible |
| Allocation during use | none | one allocation per insertion |
| Failure handling | one initial allocation | every insertion can fail |

The array version is simpler, faster and uses less memory, but it requires the
maximum number of elements to be known in advance (or a reallocation policy).
The linked version supports a truly unbounded heap, at the cost of pointer
chasing and per-node allocation.

### In-place heap sort versus extracting into a second array

Extracting a min-heap into a second array also produces a sorted result, but
it needs `O(n)` extra memory.  The in-place variant reuses the input array: it
builds a max-heap and places each maximum at the end of the shrinking prefix.
The result is sorted ascending and uses `O(1)` extra memory.

---

## Build, run and test

```bash
make -C tp02
./tp02/bin/tas
./tp02/bin/tas --display tree
./tp02/bin/tas --random 20 42
./tp02/bin/tas --sort 7 3 9 1 5
python3 -m pytest tests/test_tp02.py -q
```

The tests check the statement example, the three display modes, reproducible
random generation, sorted extraction, duplicates, negative values, a
single-element heap and a 200-element random sort.
