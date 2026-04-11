# TP 10 — Shared memory

| Program | Purpose |
|---|---|
| `bin/init [capacity] [producers]` | create the three shared segments and their semaphores |
| `bin/clean` | remove every segment and semaphore |
| `bin/rdv_n N` | N-way rendezvous (launch it N times) |
| `bin/prod [items]` | bounded FIFO producer |
| `bin/conso` | bounded FIFO consumer |
| `bin/p0`, `bin/p1`, `bin/p2` | three-process value exchange |

```bash
make
./bin/init 5 2
./bin/rdv_n 3 & ./bin/rdv_n 3 & ./bin/rdv_n 3
wait
./bin/prod 20 & ./bin/prod 20 & ./bin/conso & ./bin/conso
wait
./bin/p1 & ./bin/p2 & ./bin/p0
wait
./bin/clean
```

The N-way rendezvous uses one mutex, a shared counter and two turnstiles
(`RDV_TURNSTILE_0/1`).  The generation parity selects the turnstile, so the
same barrier can be reused several times.

The producer/consumer buffer is a circular array of `capacity` integers with
one mutex, one `empty` and one `full` semaphore.  `init` also records the
expected number of producers so that the last one can mark the buffer as
finished and wake every consumer.

The exchange uses three cells and three semaphores.  P1 writes its maximum in
cell 0, P2 writes its minimum in cell 1, and P0 swaps them (or writes `-1` in
cell 2 when no swap is needed).  P1 and P2 replace their extremum with the
value returned by P0 and repeat until the tables are separated.
