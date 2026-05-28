# TP 14 — Threads, suite

## Gestionnaire de billes (`bin/billes`)

```bash
make
./bin/billes 4 9 3 4,3,5,2
```

N threads workers demandent répétitivement `k` billes, les utilisent pendant un
délai aléatoire puis les rendent. Le thread contrôleur vérifie
`0 <= available <= maximum` pendant que les workers tournent.

La variable de condition `condition` est utilisée dans les deux sens :
`demander(k)` attend tant que `available < k`, et `rendre(k)` remet les billes et
diffuse. Un worker réveillé revérifie le prédicat, donc une diffusion ne garantit
pas que suffisamment de billes sont disponibles ; elle indique seulement que
l'état a changé.

Le scénario par défaut est celui de l'énoncé : quatre workers demandant 4, 3, 5
et 2 billes avec un maximum de 9. Comme 4 + 3 + 5 + 2 > 9, certains workers
doivent attendre.

## Pi par Monte-Carlo (`bin/pi`)

```bash
./bin/pi 1000000 4
./bin/pi --compare 1000000 4
```

Les variantes mono-thread et multi-thread tirent des points aléatoires dans le
carré unité et comptent ceux situés à l'intérieur du quart de cercle. Avec N
points, `pi ≈ 4 * hits / N`.

Chaque thread possède son propre état xorshift, donc les threads ne partagent pas
de générateur aléatoire (ce qui nécessiterait un verrou et sérialiserait le
calcul). Les nombres d'échantillons par thread diffèrent d'au plus un.
`--compare` exécute les deux variantes et affiche l'accélération mesurée.

L'estimation converge lentement : l'erreur standard diminue comme
`1 / sqrt(N)`, donc un million de points donne généralement deux ou trois
décimales exactes. Le but de l'exercice est la parallélisation et la comparaison
des temps, pas une valeur de pi à haute précision.
