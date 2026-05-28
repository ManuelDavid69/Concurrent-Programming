# TP 2 — Tas binaire et tri par tas

`bin/tas` construit un tas-min par insertions successives, l'affiche dans trois
formats et extrait chaque élément par ordre croissant. `--sort` utilise un tri
par tas-max en place sur les mêmes valeurs.

```bash
make
./bin/tas
./bin/tas --display tree
./bin/tas --random 15 42
./bin/tas --sort 7 3 9 1 5
```

Le tas est stocké dans un tableau : l'enfant gauche de l'indice `i` est
`2i + 1`, l'enfant droit est `2i + 2`, et le parent est `(i - 1) / 2`.
L'insertion ajoute la valeur et la fait remonter ; l'extraction remplace la
racine par la dernière valeur et la fait descendre. Les deux opérations sont en
`O(log n)`, et construire le tas par insertions est en `O(n log n)`. Le tri par
tas est en place et s'exécute en `O(n log n)` dans tous les cas.
