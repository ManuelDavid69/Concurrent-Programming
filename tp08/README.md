# TP 8 — System V semaphores

| Program | Purpose |
|---|---|
| `bin/init` | create the semaphore set (nine semaphores) and the shared state |
| `bin/clean` | remove both resources |
| `bin/first`, `bin/second` | strict precedence between two unrelated processes |
| `bin/rdv2` | rendezvous for two processes (`rdv2 0` and `rdv2 1`) |
| `bin/rdv3` | rendezvous for three processes (`rdv3 0`, `1`, `2`) |
| `bin/emetteur`, `bin/recepteur` | one emitter waits for two receivers |

```bash
make
./bin/init
./bin/first & ./bin/second
wait
./bin/rdv2 0 & ./bin/rdv2 1
wait
./bin/rdv3 0 & ./bin/rdv3 1 & ./bin/rdv3 2
wait
./bin/clean
```

The semaphore set contains:

| Index | Semaphore | Initial value | Role |
|---:|---|---:|---|
| 0 | `SEM_FIRST` | 0 | `second` waits for `first` |
| 1–2 | `SEM_RDV2_*` | 0 | two-process rendezvous |
| 3–5 | `SEM_RDV3_*` | 0 | three-process rendezvous |
| 6 | `SEM_MUTEX` | 1 | protects the shared counters |
| 7 | `SEM_EMETTEUR` | 0 | blocked emitters |
| 8 | `SEM_RECEPTEUR` | 0 | blocked receivers |

The shared segment only stores the number of waiting emitters, the number of
waiting receivers and a message counter.  `init` removes any stale resources
before creating them, so a crashed test does not poison the next run.

`rdv2` and `rdv3` use one semaphore per participant: each process posts the
semaphores of the others, then waits on its own.  This generalises to N
participants as long as N is known in advance; the TP 10 barrier removes that
limitation with a shared counter.
