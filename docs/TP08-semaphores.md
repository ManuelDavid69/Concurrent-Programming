# TP 8 — System V semaphores

## Objectives

- create, initialise and remove a System V semaphore set;
- use P (wait) and V (post) operations to enforce precedence;
- implement a rendezvous for two and three processes;
- implement a 1-emitter / 2-receivers rendezvous with shared counters;
- discuss how to generalise a rendezvous to N processes.

---

## 1. The semaphore set

`init` removes any stale resource, then creates one set of nine semaphores and
one shared-memory segment.

| Index | Semaphore | Initial | Role |
|---:|---|---:|---|
| 0 | `SEM_FIRST` | 0 | `second` waits for `first` |
| 1 | `SEM_RDV2_0` | 0 | rendezvous 2, process 0 |
| 2 | `SEM_RDV2_1` | 0 | rendezvous 2, process 1 |
| 3 | `SEM_RDV3_0` | 0 | rendezvous 3, process 0 |
| 4 | `SEM_RDV3_1` | 0 | rendezvous 3, process 1 |
| 5 | `SEM_RDV3_2` | 0 | rendezvous 3, process 2 |
| 6 | `SEM_MUTEX` | 1 | protects the shared counters |
| 7 | `SEM_EMETTEUR` | 0 | emitters waiting for two receivers |
| 8 | `SEM_RECEPTEUR` | 0 | receivers waiting for a rendezvous |

`clean` removes both resources.  Running `init` before every test guarantees a
known state even after a crash.

The shared segment contains:

```c
typedef struct {
    int emetteurs_attente;
    int recepteurs_attente;
    int messages;
} Etat;
```

The counters are protected by `SEM_MUTEX`; the semaphores are used for
blocking, not for busy waiting.

> **Why no `SEM_UNDO`?**  `SEM_UNDO` is useful for a lock held by a process
> that may crash.  For a semaphore used as a **signal**, it is dangerous: if a
> process posts and exits before the waiter consumes the token, the kernel
> undoes the post.  Here the programs are short-lived and the resources are
> reset by `init`, so the wrappers use plain `semop` without `SEM_UNDO`.

---

## 2. Precedence (`first`, `second`)

`first` prints, sleeps between 3 and 5 seconds (scaled in tests), prints again,
then posts `SEM_FIRST`.  `second` prints, waits on `SEM_FIRST`, prints that
`first` has finished, sleeps 1 to 4 seconds, and prints its final message.

There is no parent/child relationship, and the launch order does not matter:

- if `second` starts first, it blocks on the semaphore;
- if `first` starts first, it posts before `second` reaches the wait, and the
  semaphore keeps the token.

The semaphore therefore acts as a one-shot event, not as a lock.

---

## 3. Rendezvous for two and three processes

For N participants, the implementation creates one semaphore per participant.
Participant `i`:

1. simulates some work;
2. posts the semaphore of every other participant;
3. waits on its own semaphore.

For N = 2:

```
P0: post(S1), wait(S0)
P1: post(S0), wait(S1)
```

For N = 3, each process posts the two other semaphores and waits on its own.
This works because each post is a token that survives until the target waits,
and no process can pass its own wait before all the others have posted.

The model requires N to be known when the semaphores are created.  It also
uses `N` semaphores and `N` posts per process, i.e. `O(N^2)` posts in total.
TP 10 replaces it with a barrier based on a shared counter, one mutex and two
turnstiles, which works for any N supplied at run time and uses `O(1)` posts
per process.

---

## 4. Rendezvous 1 emitter / 2 receivers

The emitter must wait until **two** receivers are present.  A receiver must
wait until there is an emitter **and** another receiver.  The state is:

```c
int emetteurs_attente;
int recepteurs_attente;
```

### Emitter

```
lock(mutex)
if (recepteurs_attente >= 2) {
    recepteurs_attente -= 2;
    immediate = true;
} else {
    emetteurs_attente++;
    immediate = false;
}
unlock(mutex)

if (immediate)
    post(recepteur); post(recepteur);
else
    wait(emetteur);
```

### Receiver

```
lock(mutex)
if (emetteurs_attente >= 1 && recepteurs_attente >= 1) {
    emetteurs_attente--;
    recepteurs_attente--;
    immediate = true;
} else {
    recepteurs_attente++;
    immediate = false;
}
unlock(mutex)

if (immediate)
    post(emetteur); post(recepteur);
else
    wait(recepteur);
```

The second receiver is the one that completes the trio: it consumes the
waiting emitter and the waiting receiver, then wakes both.  If the emitter
arrives last and finds two receivers, it wakes both directly.

Two semaphores are enough because the counters, protected by the mutex, record
exactly how many processes are blocked; the semaphores only provide the
passive wait and the wake-up.

---

## Build, run and test

```bash
make -C tp08
./tp08/bin/init
./tp08/bin/first & ./tp08/bin/second
wait
./tp08/bin/rdv2 0 & ./tp08/bin/rdv2 1
wait
./tp08/bin/rdv3 0 & ./tp08/bin/rdv3 1 & ./tp08/bin/rdv3 2
wait
./tp08/bin/recepteur & ./tp08/bin/emetteur & ./tp08/bin/recepteur
wait
./tp08/bin/clean
python3 -m pytest tests/test_tp08.py -q
```

The tests verify that `second` is really blocked before `first` posts, that the
two- and three-process rendezvous complete, and that the emitter is released
both when it arrives before the receivers and when it arrives after them.
