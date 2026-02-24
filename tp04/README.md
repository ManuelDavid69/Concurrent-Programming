# TP 4 — Arguments, fork and wait

| Program | Purpose |
|---|---|
| `bin/miroir` | reverse every argument |
| `bin/moyenne` | average of integer grades in `[0, 20]` |
| `bin/boucle` | three `fork()` calls in a loop, without any condition |
| `bin/wait` | create N children, wait for each and print its exit code |
| `bin/enfants` | four children printing `1..200`, concurrently or in order |
| `bin/analyse-*` | the three code-analysis cases |

```bash
make
./bin/miroir trace soda saper
./bin/moyenne 12 18 3
./bin/boucle
./bin/wait 4
./bin/enfants
./bin/enfants --ordered
```

`boucle` creates a complete binary tree of processes: one process at
iteration 0, two at iteration 1, four at iteration 2, hence seven displayed
lines and eight processes.  `enfants --ordered` still creates the four
children immediately, but releases them one at a time through a pipe, which
guarantees the order `1..200` without serialising the process creation.

The analysis of the three code snippets is in [`analyse/README.md`](analyse/README.md).
