# TP 5 — `fork`, `exec` and bounded parallelism

## Objectives

- replace the current program with another one using `fork` + `execvp`;
- understand the difference between `&` (parallel) and `;` (sequential);
- keep a bounded number of child processes active;
- build a miniature `make` with parallel compilation and a conditional link.

---

## 1. Simultaneous and successive commands

A shell command is executed by forking and calling `execvp` in the child.  The
parent waits for the child with `waitpid`.

### `simultane` — `du -sh /usr/share & ls -l & ps`

All three children are created before the first `waitpid`.  The parent prints
`Lancé` for each pid, then waits in a fixed order and prints `Terminé`.  The
three commands genuinely overlap.

`SIMULTANE_PATH` replaces `/usr/share` in the `du` command; the default is the
path from the statement.  The tests use a small temporary directory so that
they do not depend on the size of `/usr/share` on the machine.

### `successif` — `du -sh /usr/share ; ls -l ; ps`

The loop forks one child, waits for it, prints the result, and only then starts
the next command.  The output is strictly `Lancé`, `Terminé`, `Lancé`, ....

`execvp` searches the `PATH`, which avoids hard-coding `/usr/bin/du` and
friends.  If `execvp` fails, the child prints the error and exits with a
non-zero status; the parent reports `Échec`.

---

## 2. Thumbnails with at most eight conversions

`miniature` walks the source directory with `opendir`/`readdir`, keeps only
regular image names (`.jpg`, `.png`, `.ppm`, `.bmp`, `.gif`, `.tif`, `.webp`),
and builds the source and destination paths.

For each image it forks a child that executes:

```
convert source -resize 10% destination
```

At most **eight** children are active.  When the pool is full:

```c
while (active >= MAX_CONVERSIONS) {
    report_finished(running, MAX_CONVERSIONS, &active, &failed);
}
```

`report_finished` calls `waitpid(-1, ...)`, finds the matching slot by pid,
prints the file name and updates the active count.  The slot is then reused by
the next task.  At the end the remaining children are drained in the same way.

The destination directory is created if needed.  A failing `convert` produces
`Échec conversion: file (code ...)` and the program returns 1, but the other
conversions are still awaited.

---

## 3. Parallel downloads with at most four `wget`

The same pool pattern is used with `MAX_DOWNLOADS = 4`.  The last argument is
the destination directory; every preceding argument is a URL.  Each child
executes:

```
wget -q -P destination URL
```

`-q` keeps the output readable, `-P` selects the destination directory.  The
parent prints `Début téléchargement` and `Téléchargement terminé` with the URL
and pid.  The bounded pool prevents a long list of URLs from spawning an
unlimited number of processes.

---

## 4. Miniature `make`

`make.c` takes one or more `.c` files followed by the executable name:

```
./bin/make main.c mathutils.c message.c program
```

1. For every source, a child executes
   `gcc -Wall -Wextra -g -c source -o basename.o`.
2. The parent waits for every compilation, printing success or failure.
3. If any compilation failed, the link is cancelled and the program returns 1.
4. Otherwise a last child executes `gcc -o executable objects...`.

Warnings are displayed but are not errors, as required by the statement.  The
failure samples cover:
- a syntax error (`bad_syntax.c`);
- a missing header (`bad_include.c`);
- an undefined symbol detected at link time (`missing_symbol.c`).

Object files are written in the current directory.  A production build system
would put them in a dedicated directory and use timestamps to avoid
recompiling unchanged files; the exercise focuses on the process management.

---

## Build, run and test

```bash
make -C tp05
./tp05/bin/simultane
./tp05/bin/successif
./tp05/bin/miniature images /tmp/thumbs
./tp05/bin/telecharge https://example.com/a.txt https://example.com/b.txt /tmp/downloads
./tp05/bin/make tp05/samples/success/*.c /tmp/demo
python3 -m pytest tests/test_tp05.py -q
```

The tests check the ordering of the successive commands, the bounded number of
simultaneous conversions and downloads (with a fake `convert` and a local HTTP
server), and both the successful and failing paths of the mini `make`.
