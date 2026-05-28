# TP 1 — Révision du C

## Objectifs

- se remettre à niveau sur l'allocation dynamique, les tableaux et l'analyse des arguments de la ligne de commande en C ;
- implémenter quelques algorithmes classiques avec une gestion soignée des cas limites ;
- compiler avec `-Wall -Wextra -g` et garder les programmes sans avertissement.

La séance contient quatre programmes indépendants.

---

## 1. Factorisation en nombres premiers (`facteurs`)

`smallest_divisor(n)` renvoie le plus petit diviseur de `n` supérieur à un, ou `n` lorsque `n` est premier. Les nombres pairs sont traités immédiatement, puis seuls les candidats impairs sont testés. La boucle est écrite

```c
for (divisor = 3; divisor <= number / divisor; divisor += 2)
```

plutôt que `divisor * divisor <= number` : la forme avec division ne peut pas déborder lorsque `number` est proche de `LLONG_MAX`.

La factorisation extrait répétitivement le plus petit diviseur. Comme le diviseur est toujours premier, la sortie est la factorisation en nombres premiers par ordre croissant. Pour l'exemple de l'énoncé :

```
1170701532 = 2 x 2 x 3 x 3 x 3 x 7 x 7 x 7 x 11 x 13 x 13 x 17
```

Le pire cas est un nombre premier, avec `O(sqrt(n))` divisions d'essai. L'utilisation mémoire est `O(1)`.

`0` et `1` n'ont pas de facteur premier et sont affichés tels quels ; un nombre négatif commence par le facteur `-1`.

---

## 2. Suite « look-and-say » (`enigme`)

Une ligne est stockée dans une `Sequence` extensible. Une étape parcourt la ligne courante, détecte les suites maximales de valeurs égales et écrit `(longueur de la suite, valeur)` pour chaque suite. Par exemple `1 2 1 1` devient `1 1 1 2 2 1`, puis `3 1 2 2 1 1`.

Le nombre de lignes et la ligne initiale sont des arguments de la ligne de commande :

```bash
./bin/enigme 3 7
./bin/enigme "1 2 1 1" 3
```

Chaque ligne est construite en `O(length)` et les longueurs croissent exponentiellement dans le pire cas, donc le coût total est proportionnel à la taille totale de la sortie.

---

## 3. Intersection de deux droites (`intersection`)

Pour deux points `P1(x1, y1)` et `P2(x2, y2)`, la pente est

```
a = (y2 - y1) / (x2 - x1)
b = y1 - a * x1
```

Le programme sépare volontairement le cas vertical (`x1 == x2`), où la pente n'est pas définie, au lieu de diviser par zéro.

| Cas | Résultat |
|---|---|
| les deux droites verticales, `x` différents | parallèles, pas d'intersection |
| les deux droites verticales, même `x` | confondues, une infinité d'intersections |
| une seule verticale | substituer `x` dans l'autre équation |
| même pente, même ordonnée à l'origine | confondues |
| même pente, ordonnée à l'origine différente | parallèles |
| sinon | `x = (b2 - b1) / (a1 - a2)`, puis `y = a1 x + b1` |

Les comparaisons en virgule flottante utilisent un epsilon relatif (`1e-9`), afin que des droites presque parallèles ne soient pas prises pour des droites parallèles à cause des arrondis.

L'énoncé travaille avec des **segments** bien qu'il dérive des équations de droites. Le programme vérifie donc aussi si l'intersection se trouve à l'intérieur des deux segments et affiche si c'est le cas.

```bash
./bin/intersection 0 0 4 4 0 4 4 0
# Intersection des droites: (2.000000, 2.000000)
# Le point appartient aux deux segments
```

---

## 4. Tri par insertion (`tri_insertion`)

L'invariant avant l'itération `i` est : `values[0..i-1]` est trié. La clé `values[i]` est sauvegardée, chaque élément plus grand du préfixe est décalé d'une case vers la droite, et la clé est insérée dans le trou.

```c
for (index = 1; index < length; index++) {
    int key = values[index];
    long position = (long)index - 1;
    while (position >= 0 && values[position] > key) {
        values[position + 1] = values[position];
        position--;
    }
    values[position + 1] = key;
}
```

- meilleur cas (déjà trié) : `n - 1` comparaisons, `Theta(n)` ;
- pire cas (trié en ordre inverse) : `n(n-1)/2` comparaisons et décalages, `Theta(n^2)` ;
- en place, `O(1)` de mémoire supplémentaire.

`--verbose` affiche le tableau après chaque insertion, ce qui rend l'algorithme facile à suivre sur l'exemple du tableau.

---

## Compilation, exécution et tests

```bash
make -C tp01
./tp01/bin/facteurs 1170701532
./tp01/bin/enigme 3 7
./tp01/bin/intersection 0 0 4 4 0 4 4 0
./tp01/bin/tri_insertion -v 5 2 4 6 1 3
python3 -m pytest tests/test_tp01.py -q
```

Les tests couvrent l'exemple de l'énoncé, les entrées premières, `0`, `1`, les nombres négatifs, les lignes exactes de look-and-say, les droites parallèles/confondues/verticales, les intersections à l'extérieur des segments, les doublons, les valeurs négatives et la trace détaillée du tri par insertion.
