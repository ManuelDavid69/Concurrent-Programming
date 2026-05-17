# TP 12 — POSIX threads

## Objectives

- create and join POSIX threads;
- implement a rendezvous with a mutex and a condition variable, then with a
  barrier;
- implement a bounded producer/consumer with condition variables;
- adapt the three-process exchange to three threads;
- parallelise the Mandelbrot computation and measure the speed-up.

Threads share the address space: the data is directly accessible, but every
shared mutable variable must be protected.  A condition variable always comes
with a mutex, and the predicate is checked in a `while` loop (not an `if`).

---

## 1. Rendezvous without a barrier (`rendezvous_sem`)

```c
lock(mutex);
generation = state->generation;
state->arrived++;
if (state->arrived == parties) {
    state->arrived = 0;
    state->generation++;
    broadcast(condition);
} else {
    while (generation == state->generation)
        wait(condition, mutex);
}
unlock(mutex);
```

The generation counter avoids the classic bug where a fast thread arrives for
the next round and is woken by the broadcast of the previous one.  A thread
only leaves when the generation it observed has changed, which means that the
last participant of its round has arrived.

`parties` is passed on the command line; the same program works for any number
of threads.

---

## 2. Rendezvous with a barrier (`rendezvous_barrier`)

`pthread_barrier_t` implements exactly the same semantics: each thread calls
`pthread_barrier_wait` and blocks until all `parties` threads have arrived.
The barrier is initialised with the number of participants and destroyed after
the joins.

The two versions produce the same output ordering: every `avant` line is
printed before any `après` line.

---

## 3. Producer/consumer (`prodcons`)

The shared buffer is a circular array of `capacity` integers.  Three
synchronisation objects are used:

```c
pthread_mutex_t mutex;
pthread_cond_t not_empty;
pthread_cond_t not_full;
```

- a producer waits while `count == capacity`, writes at `tail`, signals
  `not_empty`;
- a consumer waits while `count == 0` **and** some producer is still running,
  reads at `head`, signals `not_full`;
- the last producer increments `producers_done` and broadcasts `not_empty`,
  which lets every consumer leave once the buffer is empty.

The FIFO order follows from the circular indices.  With 4 producers, 3
consumers, a capacity of 2 and 25 items per producer, exactly 100 items are
produced and consumed.

---

## 4. Thread value exchange (`echange`)

The protocol is the same as TP 10, but the three tables and the three cells are
in the same address space:

- T1 writes its maximum in cell 0 and posts `p1_ready`;
- T2 writes its minimum in cell 1 and posts `p2_ready`;
- T0 waits for both, swaps the cells (or sets cell 2 to `-1`), and posts
  **one separate response semaphore per thread**.

Using two response semaphores is important: with a single counting semaphore
posted twice, one thread could consume both tokens and read a cell that it has
already overwritten, which breaks the protocol.  With `p0_to_p1` and
`p0_to_p2`, each thread receives exactly one response per round.

The initial tables are `{20..90}` and `{1..8}`; after eight exchanges T1 holds
`{1..8}` and T2 holds `{20..90}`.

---

## 5. Mandelbrot (`fractale`)

The image is split by rows: thread `i` handles rows `i, i + N, i + 2N, ...`.
Each thread calls the unchanged `calcul(x, y, pixel)` function; two threads
never write the same pixel, so no lock is needed.  The only shared state is the
image buffer, written at disjoint offsets.

The program prints the size, the number of threads and the wall-clock time.
On a typical machine, running the 800×600 image with 4 threads is about three
to four times faster than with 1 thread, depending on the number of cores and
on the amount of black (expensive) pixels.

![Mandelbrot rendered with four threads](images/fractale.png)

The output is deterministic: the 1-thread and 4-thread images are byte-for-byte
identical, which the test suite checks.

---

## Build, run and test

```bash
make -C tp12
./tp12/bin/rendezvous_sem 5
./tp12/bin/rendezvous_barrier 5
./tp12/bin/prodcons 4 3 2 25
./tp12/bin/echange
./tp12/bin/fractale 1 800 600 one.ppm
./tp12/bin/fractale 4 800 600 four.ppm
python3 -m pytest tests/test_tp12.py -q
```

The tests check the barrier ordering, the exact number of produced and consumed
items, the separation of the two tables, and the equality of the single- and
multi-threaded Mandelbrot images.
