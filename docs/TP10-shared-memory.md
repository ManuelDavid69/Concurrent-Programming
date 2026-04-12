# TP 10 — Shared memory

## Objectives

- create and attach System V shared-memory segments;
- build a reusable N-way barrier from a mutex, a counter and two turnstiles;
- implement a bounded FIFO producer/consumer with several producers and
  consumers;
- implement the three-process value-exchange algorithm (P0, P1, P2).

---

## 1. Resources

`init [capacity] [producers]` creates three independent groups:

| Group | Shared segment | Semaphores |
|---|---|---|
| rendezvous | `RendezVous { count, generation }` | mutex, turnstile 0, turnstile 1 |
| producer/consumer | `Buffer` with `capacity` integers | mutex, empty, full |
| exchange | `Echange { first, second, third }` | P1, P2, P0 |

`clean` removes all six resources.  Removing stale resources at the beginning
of `init` makes the programs re-runnable after a crash.

---

## 2. N-way rendezvous

Each process calls `rdv_n N`, all with the same N.  The barrier is:

```
lock(mutex)
generation = state->generation
state->count++
if (state->count == N) {
    state->count = 0
    state->generation = generation + 1
    unlock(mutex)
    post(turnstile[generation % 2]) N - 1 times
} else {
    unlock(mutex)
    wait(turnstile[generation % 2])
}
```

- the **last** process resets the counter and releases the `N-1` waiters;
- the **generation parity** selects one of two turnstiles, which lets the
  barrier be reused: a process that arrives for the next round waits on the
  other turnstile instead of consuming a token from the previous round;
- the mutex protects `count` and `generation`.

The barrier is reusable and uses a constant number of semaphores, independent
of N.  The only requirement is that every participant uses the same N.  If a
process is missing, the others block forever, which is the expected semantics
of a rendezvous.

**Answer to the statement's question.**  With the semaphores seen so far, a
rendezvous for N processes whose N is **not known when the semaphores are
created** cannot be implemented with a fixed set of per-process semaphores.
The barrier above solves it because N is passed at run time and the shared
counter records the number of arrivals.  If N were completely unknown, one
would need a dynamic resource (a shared counter and a condition-like
mechanism), or a manager process that counts arrivals.  A pure semaphore
cannot discover N by itself.

---

## 3. Bounded producer/consumer

The buffer is a circular array:

```
items[tail] = value; tail = (tail + 1) % capacity; count++
value = items[head]; head = (head + 1) % capacity; count--
```

- `empty` is initialised to `capacity`, `full` to 0;
- a producer waits on `empty` before writing, then posts `full`;
- a consumer waits on `full` before reading, then posts `empty`;
- the mutex protects `head`, `tail` and `count`.

The FIFO order follows from the circular indices: the producer writes at
`tail`, the consumer reads at `head`, and both advance in the same direction.

`init` records the expected number of producers.  Each producer registers
itself at startup (`producers_started++`, `producers_active++`).  When the last
producer finishes, it sets `finished` and posts `full` many times so that every
consumer blocked on an empty buffer wakes up, observes the flag and exits.
This avoids the classic problem of a consumer waiting forever after the last
item.

Multiple producers and consumers are supported as long as the number of
producers given to `init` matches the number actually launched.

---

## 4. Value exchange (P0, P1, P2)

P1 and P2 each hold a table of eight integers.  In each round:

1. P1 writes its **maximum** in cell 0 and posts `P1`;
2. P2 writes its **minimum** in cell 1 and posts `P2`;
3. P0 waits for both, then:
   - if `cell0 > cell1`, swaps the two values and sets cell 2 to 0;
   - otherwise sets cell 2 to `-1`;
4. P0 posts `P0` twice, releasing P1 and P2;
5. P1 replaces its maximum by the value in cell 0; P2 replaces its minimum by
   the value in cell 1; both loop.

When `cell2 == -1`, no exchange is needed: every value of P1 is smaller than
every value of P2.  P1 and P2 print their final tables and exit.

The initial tables are `{20, 30, ..., 90}` for P1 and `{1, 2, ..., 8}` for P2,
so eight exchanges are required to separate them.  The final tables are
`{1..8}` and `{20..90}`.

The three cells are protected by the protocol itself: P0 reads them only after
both posts, and P1/P2 read their cell only after P0's post.  No additional
mutex is needed.

---

## Build, run and test

```bash
make -C tp10
./tp10/bin/init 5 2
./tp10/bin/rdv_n 3 & ./tp10/bin/rdv_n 3 & ./tp10/bin/rdv_n 3
wait
./tp10/bin/prod 20 & ./tp10/bin/prod 20 & ./tp10/bin/conso & ./tp10/bin/conso
wait
./tp10/bin/p1 & ./tp10/bin/p2 & ./tp10/bin/p0
wait
./tp10/bin/clean
python3 -m pytest tests/test_tp10.py -q
```

The tests cover N = 1, 2, 3 and 5 for the barrier, FIFO order with one
producer, several producers and consumers, and the final separation of the two
tables by the exchange.
