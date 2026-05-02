# TP 14 — Threads, continued

## Ball manager (`bin/billes`)

```bash
make
./bin/billes 4 9 3 4,3,5,2
```

N worker threads repeatedly request `k` balls, use them for a random delay and
give them back.  The controller thread checks `0 <= available <= maximum`
while the workers run.

The condition variable `condition` is used by both directions:
`demander(k)` waits while `available < k`, and `rendre(k)` adds the balls back
and broadcasts.  A woken worker re-checks the predicate, so a broadcast does
not guarantee that enough balls are available; it only says that the state
changed.

The default scenario is the one from the statement: four workers asking for
4, 3, 5 and 2 balls with a maximum of 9.  Since 4 + 3 + 5 + 2 > 9, some workers
must wait.

## Monte-Carlo pi (`bin/pi`)

```bash
./bin/pi 1000000 4
./bin/pi --compare 1000000 4
```

The single-threaded and multi-threaded variants draw random points in the unit
square and count those inside the quarter circle.  With N points,
`pi ≈ 4 * hits / N`.

Each thread has its own xorshift state, so the threads do not share a random
generator (which would need a lock and would serialise the computation).  The
per-thread sample counts differ by at most one.  `--compare` runs both
variants and prints the measured speed-up.

The estimate converges slowly: the standard error decreases as
`1 / sqrt(N)`, so one million points usually give two or three correct
decimals.  The purpose of the exercise is the parallelisation and the timing
comparison, not a high-precision value of pi.
