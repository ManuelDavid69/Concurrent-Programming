# TP 7 — Pipes, continued

## Ring of processes (`bin/greater`)

```bash
make
./bin/greater 6
```

Each of the N processes draws a random value, prints it, and forwards the
largest candidate seen so far to its successor.  Process 0 starts the ring and
receives the final candidate after one complete turn.

The ring uses N pipes: pipe `i` connects process `i` to process `(i + 1) % N`.
Every process closes all descriptors except its incoming and outgoing pipe.
The final line is

```
Le plus grand nombre = 2602 - pid = 4235 - Numéro 5
```

## Calculator (`bin/calculator`)

```bash
./bin/calculator 4
CALC_SEED=42 ./bin/calculator 6
```

The server creates one request pipe (clients → server) and one response pipe
per client (server → client).  Each client sends a `Request` containing its
identifier, two operands and an operator among `+ - * /`, then waits for its
`Response`.  The server evaluates the request and writes the result to the
response pipe selected by the client identifier.

The client identifier is not the pid: an index assigned by the server is
enough and makes the response pipe lookup immediate.  All unused ends are
closed in both the server and the clients.
