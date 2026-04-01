# TP 9 — Signals

| Program | Purpose |
|---|---|
| `bin/exo1` | interrupt an infinite loop with `SIGINT` |
| `bin/exo2` | ignore `SIGINT`; stop it with `SIGKILL` |
| `bin/exo3` | clean stop through a `volatile sig_atomic_t` flag |
| `bin/exo4` | parent sends `SIGKILL` to its child after three iterations |
| `bin/exo5` | the child intercepts `SIGINT` while the parent exits |
| `bin/exo6` | `SIGUSR1`/`SIGUSR2` communication between parent and child |
| `bin/exo7` | read an integer before a five-second `SIGALRM` timeout |

```bash
make
./bin/exo1          # Ctrl+C
./bin/exo2          # Ctrl+C does nothing; use kill -KILL <pid>
./bin/exo3          # Ctrl+C performs a clean stop
./bin/exo4
./bin/exo5          # Ctrl+C: the child survives
./bin/exo6
./bin/exo7
```

Important points:

- a `volatile sig_atomic_t` flag is the only state a handler should write;
- `write` is async-signal-safe, `printf` is not;
- `sa_flags = 0` avoids `SA_RESTART`, which is what allows `fgets` to return
  and the timeout to be noticed;
- `SIGKILL` and `SIGSTOP` cannot be caught or ignored;
- signals are delivered to the foreground process group, so `Ctrl+C` reaches
  both the parent and the child in `exo5`; the parent dies, the child catches
  the signal and continues.
