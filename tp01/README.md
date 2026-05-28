# TP 1 — Remise à niveau en C

Quatre petits programmes pour réviser la mémoire dynamique, la théorie des
nombres, les tableaux et les cas limites numériques.

| Programme | Objectif |
|---|---|
| `bin/facteurs` | factorisation en nombres premiers d'un ou plusieurs entiers |
| `bin/enigme` | suite look-and-say à partir d'une première ligne arbitraire |
| `bin/intersection` | intersection de deux droites, y compris les cas vertical, parallèle et confondu |
| `bin/tri_insertion` | tri par insertion des entiers donnés sur la ligne de commande |

```bash
make
./bin/facteurs 1170701532
./bin/enigme 3 7
./bin/intersection 0 0 4 4 0 4 4 0
./bin/tri_insertion -v 5 2 4 6 1 3
```

`facteurs` évite le débordement de `d * d` en testant `d <= n / d`. `intersection`
indique si l'intersection mathématique se trouve aussi à l'intérieur des deux
segments ; il traite les droites verticales séparément au lieu de diviser par une
pente nulle.
