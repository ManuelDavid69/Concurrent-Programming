# TP 7 — Pipes, continued

## Objectives

- organise N processes in a ring of anonymous pipes;
- forward a structured message around the ring;
- design a small request/response server with one pipe per client;
- keep descriptor ownership clear in every process.

---

## 1. Ring of processes (`greater`)

### Topology

`pipe[i]` connects process `i` to process `(i + 1) % N`.  Process `i`:

- reads from `pipe[(i - 1 + N) % N]`;
- writes to `pipe[i]`.

```
process 0 -> pipe 0 -> process 1 -> pipe 1 -> ... -> process N-1 -> pipe N-1 -> process 0
```

Each process closes every pipe end except its incoming read end and its
outgoing write end.  This is what makes the ring well-defined: no process keeps
a descriptor that could prevent EOF or create a deadlock.

### Protocol

Every process draws a value and builds a `Candidate`:

```c
typedef struct {
    int value;
    pid_t pid;
    int rank;
} Candidate;
```

Process 0 sends its candidate to process 1.  Every other process reads the
incoming candidate, keeps the larger of the two (its own and the incoming one),
and forwards the winner.  After one complete turn, process 0 receives the
global maximum with the pid and rank of the process that produced it.

The program prints every generated value and then:

```
Le plus grand nombre = 2602 - pid = 4235 - Numéro 5
```

The order of the `processus ...` lines is arbitrary; the final line is printed
after the whole ring has completed.

With `N = 1`, process 0 writes to and reads from its own pipe; the result is
its own candidate.  The test suite exercises N = 1, 2, 3, 6 and 12.

---

## 2. Calculator (`calculator`)

### Topology

```
client 0 --\
client 1 ----> request pipe ----> server
client 2 --/                       |
   ^                               |
   +---- response pipe per client -+
```

There is **one** request pipe for all clients and **one response pipe per
client**.  The server creates the pipes before forking the clients, so every
client inherits the descriptors.

### Request and response

```c
typedef struct {
    int client_id;
    int left;
    int right;
    char operator;
} Request;

typedef struct {
    int left;
    int right;
    int result;
    char operator;
} Response;
```

The client identifier is an index between 0 and N-1, not a pid.  The server
uses it directly as an index into the response-pipe array, which is simpler and
avoids a search.

Each client sends exactly one request and waits for its response.  The server
reads requests in arrival order and answers immediately, so several clients
can be active at the same time.  The `Request`/`Response` structures are small
enough to be written atomically to a pipe (`PIPE_BUF`), but the code still uses
`write_all`/`read_all` for robustness.

Supported operators: `+`, `-`, `*`, `/`.  A division by zero is rejected by the
server (the result is set to 0 and an error is printed), although the clients
never generate a zero divisor.

`CALC_SEED` makes the client requests reproducible: client `i` uses
`seed + i * 7919`, so the clients do not all generate the same request.

---

## Build, run and test

```bash
make -C tp07
./tp07/bin/greater 6
CALC_SEED=42 ./tp07/bin/calculator 4
python3 -m pytest tests/test_tp07.py -q
```

The tests check that the ring returns the true maximum for several sizes,
including one process, and that every calculator response is arithmetically
correct and reproducible with a fixed seed.
