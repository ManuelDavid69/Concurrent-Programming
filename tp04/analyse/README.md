# Code analysis — answers

The three programs are compiled as `bin/analyse-cas1`, `bin/analyse-cas2`,
`bin/analyse-cas2-ordonne` and `bin/analyse-cas3`.  The full reasoning is in
[`../../docs/TP04-arguments-fork.md`](../../docs/TP04-arguments-fork.md);
this page is the short answer sheet.

## Case 1

- The `if` block runs in the **parent** (`fork()` returned a positive pid).
- Not deterministic in order.  Possible output: `8 6 4 2 0` (any permutation
  of the set `{0, 2, 4, 6, 8}` is possible in principle).
- Make it deterministic by waiting for the child before printing; the deepest
  child then prints `0` first: `0 8 6 4 2`.
- `fork()` can fail with `EAGAIN` (process limit) or `ENOMEM`.

## Case 2

- Process tree: a chain `A -> B -> C -> D -> E`.
- Not deterministic because of the random `sleep`.
- To print `E D C B A`, each parent waits for its child before printing its own
  letter (see `cas2_ordonne.c`).

## Case 3

- The initial process creates four children and waits for all of them.
- Each child sums one quarter of `tab`; the quarters sum to `100`, `125`,
  `100`, `125`.
- The order of the four sums is not deterministic.
- `Calculs terminés` is printed after the four `wait`, so it is last.

## General conclusion

`fork()` duplicates the process, including its current loop state.  Without a
condition, a loop creates an exponential number of processes.  `wait` and
`waitpid` are required to reap children and to order a parent after its
children, and scheduling still makes concurrent output non-deterministic
unless explicit synchronisation is added.
