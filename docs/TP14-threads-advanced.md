# TP 14 — Threads, continued

## Objectives

- share a limited resource between N worker threads;
- implement a blocking `demander(k)` / `rendre(k)` pair with a condition
  variable;
- supervise the shared invariant from a controller thread;
- estimate pi with the Monte-Carlo method, first single-threaded then
  multi-threaded, and compare the execution times.

---

## 1. Ball manager (`billes`)

### Shared state

```c
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int available;
    int maximum;
    int workers_done;
    int stop_controller;
    int controller_checks;
    int invariant_error;
} BallManager;
```

### Worker protocol

Each worker repeats `iterations` times:

```
demander(k)
use the k balls (random sleep)
rendre(k)
```

`demander(k)`:

```c
lock(mutex);
while (available < k)
    wait(condition, mutex);
available -= k;
unlock(mutex);
```

`rendre(k)`:

```c
lock(mutex);
available += k;
broadcast(condition);
unlock(mutex);
```

The `while` is essential: a broadcast only signals that the state changed, not
that enough balls are now available.  Several workers can wake at the same
time and only one of them may find a sufficient number of balls; the others
must go back to sleep.

The default scenario is `N = 4`, `maximum = 9` and demands `4, 3, 5, 2`.
Since `4 + 3 + 5 + 2 = 14 > 9`, the workers cannot all be satisfied at once,
which exercises the waiting path.

### Controller

The controller thread checks the invariant

```
0 <= available <= maximum
```

once per second (scaled by `PC_SLEEP_SCALE` in the tests), increments its
check counter and records a violation if the invariant is broken.  The main
thread sets `stop_controller` after joining the workers, then joins the
controller.

The mutex protects both `available` and the controller's flags; the worker
prints the remaining number of balls after taking a consistent snapshot under
the mutex.

---

## 2. Monte-Carlo estimation of pi (`pi`)

### Method

A random point `(x, y)` is drawn uniformly in `[0, 1] × [0, 1]`.  It lies in
the quarter of the unit circle when `x² + y² <= 1`.  The area of the quarter
circle is `pi / 4`, so

```
pi ≈ 4 * hits / samples
```

The standard error decreases as `1 / sqrt(N)`: one million samples typically
give two or three correct decimals.  The goal is the parallelisation, not a
record precision.

### Random generator

Each thread owns an `xorshift64*` state.  This avoids a shared `rand()` (which
would need a lock and would serialise the hot loop) and gives independent
streams.  The seeds are derived from the time, the pid and the thread index.

### Parallelisation

- `samples` is divided as evenly as possible between the threads (the first
  `samples % threads` threads get one extra point);
- each thread counts its own hits;
- the main thread sums the hits after the joins.

No mutex is needed in the hot loop: each thread writes only its own `hits`
field, and the main thread reads it after `pthread_join`.

`--compare` runs the single-threaded version, then the multi-threaded one, and
prints both estimates and the measured speed-up.  On a machine with several
cores, the multi-threaded version is typically 2–4 times faster for a large N;
the exact value depends on the scheduler and on the number of available cores.

---

## Build, run and test

```bash
make -C tp14
./tp14/bin/billes 4 9 3 4,3,5,2
./tp14/bin/pi 1000000 4
./tp14/bin/pi --compare 1000000 4
python3 -m pytest tests/test_tp14.py -q
```

The tests run the default ball scenario, check that the invariant is never
violated, verify that an impossible demand is rejected, and check that both pi
estimates are within 0.05 of the real value.
