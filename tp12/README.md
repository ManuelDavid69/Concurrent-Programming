# TP 12 — POSIX threads

| Program | Purpose |
|---|---|
| `bin/rendezvous_sem` | N-thread rendezvous with a mutex and a condition variable |
| `bin/rendezvous_barrier` | the same rendezvous with `pthread_barrier_t` |
| `bin/prodcons` | bounded producer/consumer with several threads |
| `bin/echange` | three-thread value exchange |
| `bin/fractale` | multi-threaded Mandelbrot renderer |

```bash
make
./bin/rendezvous_sem 5
./bin/rendezvous_barrier 5
./bin/prodcons 2 2 3 20
./bin/echange
./bin/fractale 4 800 600 fractale.ppm
```

`rendezvous_sem` uses a generation counter so the condition variable cannot
wake a thread from the next round.  `rendezvous_barrier` is the same algorithm
delegated to the pthread barrier.

`prodcons` uses a circular buffer, a mutex and two condition variables.  The
last producer broadcasts `not_empty` after setting `producers_done`, which lets
every consumer leave once the buffer is empty.

`echange` uses three semaphores and the same P0/P1/P2 protocol as TP 10, but
the tables and the three cells are in the same address space.  The threads
share the variables directly; only the ordering needs semaphores.

`fractale` splits the rows between N threads.  Each thread calls the `calcul`
function on its own rows, so no lock is needed: two threads never write the
same pixel.  Run it with 1 and 4 threads and compare the `time` output.
