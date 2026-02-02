# Documentation

One document per practical session. Each one follows the same structure:

1. objectives and link with the course;
2. implementation choices and program structure;
3. answers to the theoretical questions;
4. how to build, run and test the programs;
5. observed results and, where relevant, performance measurements.

| Session | Topic | Document |
|---|---|---|
| 1 | C refresher: prime factors, look-and-say, line intersection, insertion sort | [TP01-c-refresher.md](TP01-c-refresher.md) |
| 2 | Binary heaps and heap sort | [TP02-heap.md](TP02-heap.md) |
| 3 | PPM image blur and a state-machine digicode | [TP03-image-state-machine.md](TP03-image-state-machine.md) |
| 4 | Command-line arguments, `fork`, `wait` and process trees | [TP04-arguments-fork.md](TP04-arguments-fork.md) |
| 5 | `fork`/`exec`, bounded parallelism and a mini build tool | [TP05-fork-exec.md](TP05-fork-exec.md) |
| 6 | Anonymous pipes, redirections and filters | [TP06-pipes.md](TP06-pipes.md) |
| 7 | Ring of processes and a pipe-based calculator | [TP07-pipes-ring.md](TP07-pipes-ring.md) |
| 8 | System V semaphores: precedence, rendezvous and 1-to-2 rendezvous | [TP08-semaphores.md](TP08-semaphores.md) |
| 9 | Signals: interception, masking, timeout and parent/child communication | [TP09-signals.md](TP09-signals.md) |
| 10 | Shared memory: N-way rendezvous, producer/consumer and value exchange | [TP10-shared-memory.md](TP10-shared-memory.md) |
| 11 | Shared-memory calculator and parallel image processing | [TP11-shared-memory-applications.md](TP11-shared-memory-applications.md) |
| 12 | POSIX threads: rendezvous, producer/consumer, exchange and Mandelbrot | [TP12-threads.md](TP12-threads.md) |
| 14 | Threads continued: resource manager and Monte-Carlo estimation of pi | [TP14-threads-advanced.md](TP14-threads-advanced.md) |
| — | Software graphics pipeline with threads and barriers | [PIPELINE-graphics.md](PIPELINE-graphics.md) |

## Conventions

- Code, identifiers, comments and tests are in English; the course statements
  are in French and the written answers keep the French terminology in mind
  (*tube*, *sémaphore*, *mémoire partagée*, *barrière*).
- Every program is compiled with `-Wall -Wextra -g`; the CI build additionally
  uses `-Werror`.
- The tests are integration tests: they build the binaries, run them and check
  their output, exit status, generated files and, where relevant, timing or
  synchronisation properties.
