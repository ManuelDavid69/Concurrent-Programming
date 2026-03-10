# TP 6 — Anonymous pipes

| Program | Purpose |
|---|---|
| `bin/guess` | number-guessing game: parent asks, child holds the secret |
| `bin/redirection` | `wc < /etc/passwd`, `cut | tr`, and the full `cut | sed | sort > users` pipeline |
| `bin/somme` | generator + even/odd filter + two summers, connected by five pipes |

```bash
make
printf '10\n20\n30\n40\n42\n' | GUESS_SECRET=42 ./bin/guess
./bin/redirection simple
./bin/redirection pipe
./bin/redirection pipeline
./bin/somme --verbose 20 42
```

`guess` uses two pipes: one for the propositions and one for the reply
(`+`, `-` or `=`).  `GUESS_SECRET` makes the tests deterministic; without it
the child draws a random number between 1 and 1000.

`redirection` shows `dup2` on standard input/output.  The pipeline is built
with three children and two pipes; only the first child knows about
`/etc/passwd`, and only the last one knows about `users`.

`somme` closes every unused pipe end in every process, which is essential to
avoid blocking reads.  The generator sends `-1` as the end marker; the filter
forwards it to both summers, which add up their values and send one sum back
to the generator.
