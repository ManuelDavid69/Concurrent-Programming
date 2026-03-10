# TP 6 — Anonymous pipes

## Objectives

- create and use anonymous pipes (`pipe`, `read`, `write`);
- connect processes and redirect their standard descriptors (`dup2`);
- close every unused pipe end to avoid blocking reads;
- build a multi-process filter with a sentinel value.

---

## 1. The guessing game (`guess`)

The parent and the child communicate through **two** pipes:

```
parent ---- int guess ----> child
parent <--- char reply ---- child
```

The child draws a secret between 1 and 1000.  For each proposition it writes:

- `+` if the guess is smaller than the secret;
- `-` if it is larger;
- `=` if it is equal, then exits.

The parent prints the hint, reads the next line and loops until `=` or EOF.
`GUESS_SECRET` allows reproducible tests; otherwise the secret is random.

Both directions use `write_all` and `read_all`, which retry on `EINTR` and
handle partial transfers.  A single `read` is not guaranteed to return a whole
`int`, even on a pipe.

---

## 2. Redirections and pipelines (`redirection`)

### `simple` — `wc < /etc/passwd`

The child opens `/etc/passwd`, duplicates the descriptor onto standard input
with `dup2`, closes the original descriptor and executes `wc`.  `wc` does not
know where its input comes from.

### `pipe` — `cut -f 1 -d : /etc/passwd | tr a-z A-Z`

A pipe is created before the two forks.  The `cut` child redirects the write
end to stdout; the `tr` child redirects the read end to stdin.  The parent
closes both ends, then waits for both children.

### `pipeline` — the complete command

```
cut -f 1,3 -d : < /etc/passwd
  | sed 's+^\(.*\):\(.*\)+\2:\1+'
  | sort -n > users
```

Three children and two pipes:

| Process | stdin | stdout |
|---|---|---|
| `cut` | `/etc/passwd` | pipe 1 |
| `sed` | pipe 1 | pipe 2 |
| `sort` | pipe 2 | `users` |

Only `cut` opens `/etc/passwd`; only `sort` opens `users`.  This is exactly the
shell behaviour: each command sees only its own descriptors.

**Why close unused ends?**  If the parent (or another process) keeps the write
end of a pipe open, the reader never sees EOF and blocks forever once the
producer exits.  Closing every unused end is not an optimisation, it is a
correctness requirement.

---

## 3. Even/odd filter (`somme`)

The program is made of four processes and five pipes:

```
generator --numbers--> filter --even--> pair  --sum--> generator
                              \--odd---> impair --sum--> generator
```

- the **generator** (the initial process) draws N numbers, writes them to the
  filter, then writes `-1`;
- the **filter** reads each number and forwards it to the even or odd pipe;
  when it reads `-1`, it forwards `-1` to both summers and exits;
- **pair** and **impair** add their values and send one integer back to the
  generator;
- the generator reads both sums and prints the totals.

The sentinel `-1` is valid here because the generated values are non-negative.
Each process closes all the pipe ends it does not use, so the filter sees EOF
only when the generator closes its write end, and the summers terminate as
soon as they receive the sentinel.

The sum of the two partial sums is also the sum of all generated values; the
tests recompute it from the `--verbose` output.

---

## Build, run and test

```bash
make -C tp06
printf '10\n20\n30\n40\n42\n' | GUESS_SECRET=42 ./tp06/bin/guess
./tp06/bin/redirection simple
./tp06/bin/redirection pipe
./tp06/bin/redirection pipeline
./tp06/bin/somme --verbose 100 42
python3 -m pytest tests/test_tp06.py -q
```

The tests compare the redirections with the real shell commands, exercise the
guessing game with a forced secret, and check the even/odd sums for several
values of N, including the empty case.
