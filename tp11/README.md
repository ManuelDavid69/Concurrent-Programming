# TP 11 — Shared-memory applications

## Calculator (`bin/client`, `bin/serveur`)

```bash
make
./bin/init
./bin/serveur & ./bin/serveur & ./bin/serveur &
CALC_SEED=42 ./bin/client 5 & CALC_SEED=7 ./bin/client 5
wait
kill -INT %1 %2 %3       # servers stop on SIGINT
./bin/clean
```

A single request queue is shared by all clients, and each client has a result
slot protected by its own semaphore.  Several servers dequeue requests
concurrently and write the response to `results[client_id]`; the matching
result semaphore wakes the right client.

## Parallel image processing (`bin/client_image`, `bin/serveur_image`)

```bash
./bin/init
./bin/serveur_image out --drain & ./bin/serveur_image out --drain &
./bin/client_image out images/*.ppm
wait
```

The five task slots are protected by the `slots`/`items` semaphores; a server
takes one path, blurs the PPM image and writes the result to the output
directory.  Without `--drain` the servers run until `SIGINT`, as required by
the statement; `--drain` is a convenience for scripts and tests: the server
exits once the queue is empty and every client has finished.

`init` and `clean` manage the resources of both parts.  The image client must
use the same output directory as the servers.
