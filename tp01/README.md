# TP 1 — C refresher

Four small programs warming up dynamic memory, number theory, arrays and
numerical edge cases.

| Program | Purpose |
|---|---|
| `bin/facteurs` | prime factorisation of one or more integers |
| `bin/enigme` | look-and-say sequence from an arbitrary first line |
| `bin/intersection` | intersection of two lines, including vertical, parallel and confounded cases |
| `bin/tri_insertion` | insertion sort of the integers given on the command line |

```bash
make
./bin/facteurs 1170701532
./bin/enigme 3 7
./bin/intersection 0 0 4 4 0 4 4 0
./bin/tri_insertion -v 5 2 4 6 1 3
```

`facteurs` avoids `d * d` overflow by testing `d <= n / d`.  `intersection`
reports whether the mathematical intersection also lies inside both segments;
it handles vertical lines separately instead of dividing by a zero slope.
