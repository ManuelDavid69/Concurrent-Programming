# TP 9 — Signals

## Objectives

- intercept and ignore signals;
- stop a program cleanly from a handler;
- send signals between a parent and a child;
- understand process groups and terminal-generated signals;
- implement a timeout with `SIGALRM`.

A handler runs asynchronously, between two machine instructions of the main
program.  It may only call **async-signal-safe** functions and write to
`volatile sig_atomic_t` variables.  `write` is safe; `printf`, `malloc` and
`exit` are not (the examples use `write` and `_exit`).

---

## 1. Interception (`exo1`)

An infinite loop prints a message every second.  The handler for `SIGINT`
writes a message and calls `_exit(0)`.

When the program is launched in the background with `./exo1 &`, `Ctrl+C` does
not reach it: the terminal sends `SIGINT` to the **foreground process group**,
and the background job is not part of it.  Use `kill -INT <pid>` instead.

## 2. Ignoring a signal (`exo2`)

`signal(SIGINT, SIG_IGN)` makes `Ctrl+C` ineffective.  `SIGKILL` cannot be
caught, blocked or ignored, so it is the only reliable way to stop the
program.

## 3. Clean stop (`exo3`)

The handler sets a `volatile sig_atomic_t` flag.  The loop is conditioned by
that flag, so the program leaves the loop normally, flushes its output and
returns from `main`.  This is the recommended pattern when the program has
resources to release.

## 4. Parent kills its child (`exo4`)

The father prints three times, then sends `SIGKILL` to its child and waits.
The child cannot intercept `SIGKILL`; it is terminated by the kernel.  The
parent still calls `waitpid`, which collects the termination status and avoids
a zombie.

## 5. `Ctrl+C` and the hierarchy (`exo5`)

The child installs a `SIGINT` handler but the parent does not.  When `Ctrl+C`
is pressed, the terminal sends `SIGINT` to the whole foreground process group:

- the parent has the default action → it terminates;
- the child catches the signal, prints a message and continues.

The child survives its parent.  This shows that a signal is delivered to every
process of the group, and that a handler changes the default action only for
the process that installed it.

## 6. `SIGUSR1` / `SIGUSR2` (`exo6`)

The child blocks `SIGUSR1` and `SIGUSR2`, installs a single handler, signals
that it is ready through a pipe, and waits with `sigsuspend`.  The parent:

- sends `SIGUSR1` at iterations 3 and 5;
- sends `SIGUSR2` after the loop to announce the end.

A single handler receives the signal number and dispatches:

```c
if (signal_number == SIGUSR1) { /* display */ }
else if (signal_number == SIGUSR2) { running = 0; }
```

Blocking the signals and using `sigsuspend` removes the race between the
arrival of a signal and the call to `pause`.

## 7. Timeout (`exo7`)

The program arms `alarm(5)` (or `PC_TIMEOUT_SECONDS`), then reads a line with
`fgets`.  The `SIGALRM` handler only sets a flag.  Because `sigaction` is used
without `SA_RESTART`, the interrupted `fgets` returns and the program notices
the flag:

- valid integer → `alarm(0)`, print `Ok merci !!`, exit 0;
- invalid line → reprompt;
- timeout → print `Trop tard !!`, exit 1.

The read is line-based (`fgets` + strict parsing), so a non-integer input does
not crash the program.

---

## Build, run and test

```bash
make -C tp09
./tp09/bin/exo1
./tp09/bin/exo2
./tp09/bin/exo3
./tp09/bin/exo4
./tp09/bin/exo5
./tp09/bin/exo6
./tp09/bin/exo7
python3 -m pytest tests/test_tp09.py -q
```

The tests send real signals (`SIGINT`, `SIGKILL`, `SIGUSR1`, `SIGUSR2`), check
the clean exits, verify that `exo2` survives `SIGINT`, that the child of
`exo5` survives its parent, and that `exo7` accepts a valid integer or reports
the timeout.
