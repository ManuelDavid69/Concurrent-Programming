# TP 4 — Arguments, `fork` and `wait`

## Objectives

- read and validate command-line arguments;
- understand that `fork()` duplicates the current process, including its
  instruction pointer, variables and open descriptors;
- draw process trees;
- use `wait`/`waitpid` to reap children and collect exit codes;
- make a concurrent display deterministic without giving up concurrency.

---

## 1. `miroir` and `moyenne`

`miroir` reverses each argument in place with two indices.  It modifies
`argv[i]` directly, which is allowed because the strings are writable in a
normal `main`.

`moyenne` uses the strict parser from `common` and rejects anything that is
not an integer in `[0, 20]`.  In particular `3.5`, `toto` and `22` produce
`Note(s) non valide(s)`; no argument produces the message from the statement.
The average is computed as a `double` and printed with `%.2f`.

---

## 2. `fork` in a loop (`boucle`)

The program calls `fork()` three times with no `if` and no `break`.  Every
process therefore continues the loop, and the number of processes doubles at
each iteration.

```
iteration 0:            P0
                         |
iteration 1:            P0 -- C1
                         |      |
iteration 2:      P0 -- C2     C1 -- C3
```

The **process tree** is a complete binary tree of depth 3: 8 processes in
total.  Each process prints at every iteration it executes:

| Created at | Prints at | Number of lines |
|---|---|---|
| before the loop (root) | 0, 1, 2 | 3 |
| iteration 0 (1 process) | 0, 1, 2 | 3 |
| iteration 1 (2 processes) | 1, 2 | 4 |
| iteration 2 (4 processes) | 2 | 4 |
| **total** | | **14** |

The exact interleaving of the 14 lines depends on the scheduler.  The root's
lines have the same pid and different loop counters; a child's first line has
the same counter as the iteration during which it was forked, and its `fork`
return value is `0`.

`wait(NULL)` at the end only reaps the process's own children; every process
runs that loop, so the whole tree is reaped.

---

## 3. `wait`

The parent creates all N children first, then calls `waitpid(-1, ...)` in a
loop.  Each child:

- prints its pid and its parent's pid;
- sleeps `2 * i` seconds (scaled by `PC_SLEEP_SCALE` in the tests);
- prints that it resumes;
- exits with code `i`.

The parent prints the pid and the exit status of each child as soon as one
terminates.

**Is the program deterministic?**  The set of messages and exit codes is
deterministic, but the order of the parent's `wait` returns is not guaranteed:
it depends on scheduling, even though the increasing sleeps make the order
`0, 1, 2, ...` very likely.  To make it fully deterministic, the parent could
wait for a specific pid (`waitpid(children[i], ...)`) instead of `-1`.

---

## 4. Four children printing `1..200`

### Variant 1 — concurrent

Each child prints its own block of 50 numbers.  The blocks are correct but
their lines can interleave arbitrarily:

```
1
51
2
52
...
```

### Variant 2 — guaranteed order

The four children are still created immediately, but each one blocks on a
private pipe.  The parent writes one token to child 0, waits for it, writes a
token to child 1, and so on.  The output is exactly `1, 2, ..., 200`, while the
process creation remains concurrent.

An alternative would be to let each child write into a pipe and have the
parent concatenate the pipes in order.  The token solution uses less memory
and demonstrates a one-byte synchronisation message.

---

## 5. Code analysis

### Case 1

```c
for (i = 1; i < 5; i++) {
    fils_pid = fork();
    if (fils_pid > 0) {
        n = i * 2;
        break;
    }
}
printf("%d\n", n);
```

- In the `if` block we are in the **parent** (`fork()` returned a positive
  pid).  The parent breaks out of the loop; the child continues with the next
  iteration.
- The program is **not deterministic** in its output order.  The set of
  printed values is `{0, 2, 4, 6, 8}`: the deepest child (created at `i = 4`)
  leaves the loop with `n = 0`; its ancestors print `8`, `6`, `4`, `2`.
  One possible output is `0 8 6 4 2`, another is `8 0 6 4 2`.
- To make it deterministic, a parent can wait for its child before printing:

  ```c
  if (fils_pid > 0) {
      n = i * 2;
      wait(NULL);
      break;
  }
  ```

  With this change the deepest child prints first and the output is
  `0, 8, 6, 4, 2`.
- `fork()` can fail: `EAGAIN` when the process limit is reached, `ENOMEM`
  when the kernel cannot allocate the process structures.  It returns `-1`
  and sets `errno`; the program should test this case.

### Case 2

```c
for (i = 0; i < 4; i++)
    if (fork())
        break;
/* sleep and print 'A' + i */
```

- Process tree: a **chain** `A -> B -> C -> D -> E`.  At each iteration the
  parent breaks (and will print its letter), while the child continues the
  loop.  The last child leaves the loop with `i = 4` and prints `E`.
- The program is **not deterministic**: each process sleeps for a random
  number of seconds, so the order of the five lines varies.
- To obtain `E D C B A` without changing the first lines, each parent must
  wait for its child before printing:

  ```c
  pid_t child = fork();
  if (child > 0) {
      waitpid(child, NULL, 0);
      printf("Je suis %c\n", 'A' + i);
      break;
  }
  ```

  The leaf `E` prints first, then its parent `D`, and so on up to `A`.
  The complete variant is in `analyse/cas2_ordonne.c`.

### Case 3

- Process tree: the initial process creates four children and then waits for
  them.  Each child keeps the value of `i` at the moment of its `fork`, breaks
  out of the loop, and sums one quarter of `tab`.
- The parent's `Calculs terminés` is printed **after** the four `wait`, so it
  is the last line.
- The order of the four `i somme` lines is **not deterministic**.
- The quarters are `100`, `125`, `100`, `125` for `i = 0, 1, 2, 3`
  (`tab[j] = j % 10`).
- Conclusions: `fork` duplicates the whole process; a condition is needed to
  give each process a role; `wait` is needed to avoid the parent finishing
  before its children; and scheduling makes concurrent output
  non-deterministic unless explicit synchronisation is added.

---

## Build, run and test

```bash
make -C tp04
./tp04/bin/miroir trace soda saper
./tp04/bin/moyenne 12 18 3
./tp04/bin/boucle
./tp04/bin/wait 4
./tp04/bin/enfants
./tp04/bin/enfants --ordered
./tp04/bin/analyse-cas1
./tp04/bin/analyse-cas2
./tp04/bin/analyse-cas2-ordonne
./tp04/bin/analyse-cas3
python3 -m pytest tests/test_tp04.py -q
```

The tests check the exact messages, the 8-process tree of `boucle`, the 200
values of both `enfants` variants, the exit codes collected by `wait`, and the
answers of the three analysis programs.
