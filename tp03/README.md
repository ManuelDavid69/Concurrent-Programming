# TP 3 — Traitement d'images et automates

## Flou (`bin/flou`)

```bash
make
./bin/flou sample.ppm out.ppm
```

Le programme lit un fichier PPM P3 ou P6, applique un filtre moyen 3×3 et écrit
un fichier P6. Les pixels de bord utilisent la stratégie de *kernel crop* : seuls
les voisins qui existent réellement sont moyennés, de sorte que la sortie
conserve les dimensions de l'entrée.

## Digicode (`bin/digicode`)

```bash
printf '1235742' | ./bin/digicode 5742
printf '78267B' | ./bin/digicode 8267B
printf '7A8267B' | ./bin/digicode 8267B
./bin/digicode 5551 --trace
```

Les deux premières variantes utilisent un automate KMP, qui gère correctement
les chiffres répétés (`5551`). La troisième variante accumule les chiffres entre
deux lettres de validation : `7A8267B` est valide car la première tentative est
rejetée, tandis que `78267B` ne l'est pas car le `7` initial appartient à la même
tentative.

`bin/table` affiche les tables de transition utilisées par les automates.
