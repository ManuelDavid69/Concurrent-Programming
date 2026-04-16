# TP 11 — Shared-memory applications

## Objectives

- build a multi-server calculator with shared memory and semaphores;
- support several clients sending requests concurrently;
- return each result to the right client without copying the whole request back;
- process a batch of images in parallel with a pool of worker processes.

---

## 1. Shared-memory calculator

### Data layout

```c
typedef struct {
    int client_id;
    int left;
    int right;
    char operator;
} CalcRequest;

typedef struct {
    int left;
    int right;
    int result;
    char operator;
} CalcResponse;

typedef struct {
    CalcRequest queue[16];
    CalcResponse results[8];
    int head, tail, count;
    int client_pid[8];
} CalcShared;
```

One request queue is shared by every client.  Each client also owns a result
slot and a private semaphore (`CALC_RESULT_BASE + slot`).

### Semaphores

| Semaphore | Initial | Role |
|---|---:|---|
| `CALC_MUTEX` | 1 | protects the queue, the counters and the results |
| `CALC_SLOTS` | 16 | free places in the request queue |
| `CALC_ITEMS` | 0 | requests waiting to be processed |
| `CALC_RESULT_BASE + i` | 0 | result ready for client slot `i` |

### Client

1. acquire a free slot under the mutex (`client_pid[i] == 0`);
2. repeat `requests` times:
   - wait for a free queue slot, append the request, post `items`;
   - wait on its result semaphore;
   - read `results[slot]` under the mutex and print it;
3. release the slot.

The client sends one request and waits for its result before sending the next,
as required by the statement.

### Server

The server loops:

1. wait for `items`;
2. dequeue under the mutex, post `slots`;
3. evaluate, write `results[client_id]` under the mutex, post the client's
   result semaphore.

Several servers can run at the same time; the queue is the only shared
producer/consumer structure, and the mutex makes dequeue atomic.  A `SIGINT`
handler sets a flag; the server uses `sem_p_interruptible` so the blocking wait
returns with `EINTR` and the loop can exit cleanly.

### Why a client identifier?

The result could be sent through a pipe, but here the server needs a fast way
to find the client's result zone.  The slot index (`0..7`) is that key.  It is
stable even if the pid is reused by the operating system, and it is smaller
than a pid.

---

## 2. Parallel image processing

### Data layout

```c
typedef struct {
    int head, tail, count;
    int active_clients;
    int finished;
    char tasks[5][10000];
} ImageShared;
```

Five task slots, each able to hold a path of up to 10 000 characters.  A path
cannot be transmitted as a pointer: the client and the servers are different
processes, so the characters must be copied into the shared segment.

### Protocol

- `IMG_SLOTS` (initial 5) and `IMG_ITEMS` (initial 0) implement the bounded
  queue;
- the client copies each path into `tasks[tail]`, advances `tail`, posts
  `items`;
- a server waits for an item, copies `tasks[head]` into a local buffer,
  advances `head`, posts `slots`, then processes the file;
- the client increments `active_clients` when it starts and decrements it when
  it has enqueued everything; the last client sets `finished`.

The servers in normal mode run until `SIGINT`, as required.  `--drain` is a
convenience for scripts and tests: the server polls the queue and exits once
`finished` is set and the queue is empty.  A server that has already dequeued a
task finishes that task before exiting.

### Processing

The server loads the PPM file with the same `image_load` used in TP 3, applies
the 3×3 kernel-crop blur and saves the result as P6 under the output directory.
It prints the source and destination paths.

### Results

With five 4×4 images and two servers, all five outputs are produced.  The
servers may split the work unevenly (one can process all five if it is
scheduled first); the requirement is that every file is processed exactly
once and that the output directory contains exactly the expected files.

---

## Build, run and test

```bash
make -C tp11
./tp11/bin/init
./tp11/bin/serveur & ./tp11/bin/serveur &
CALC_SEED=42 ./tp11/bin/client 5
kill -INT %1 %2
./tp11/bin/serveur_image out --drain & ./tp11/bin/serveur_image out --drain &
./tp11/bin/client_image out images/*.ppm
wait
./tp11/bin/clean
python3 -m pytest tests/test_tp11.py -q
```

The tests start two calculator servers, run two clients concurrently, verify
the arithmetic of every response and the clean `SIGINT` shutdown, then process
five generated PPM images with two drain-mode servers and check every output.
