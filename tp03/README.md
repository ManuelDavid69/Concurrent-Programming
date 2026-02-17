# TP 3 — Image processing and state machines

## Blur (`bin/flou`)

```bash
make
./bin/flou sample.ppm out.ppm
```

The program reads a P3 or P6 PPM file, applies a 3×3 mean filter and writes a
P6 file.  Border pixels use the *kernel crop* strategy: only the neighbours
that actually exist are averaged, so the output keeps the input dimensions.

## Digicode (`bin/digicode`)

```bash
printf '1235742' | ./bin/digicode 5742
printf '78267B' | ./bin/digicode 8267B
printf '7A8267B' | ./bin/digicode 8267B
./bin/digicode 5551 --trace
```

The first two variants use a KMP automaton, which handles repeated digits
correctly (`5551`).  The third variant accumulates digits between two
validation letters: `7A8267B` is valid because the first attempt is rejected,
while `78267B` is not because the initial `7` belongs to the same attempt.

`bin/table` prints the transition tables used by the automata.
