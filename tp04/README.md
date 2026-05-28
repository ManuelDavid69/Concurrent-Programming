# TP 4 — Arguments, fork et wait

| Programme | Objectif |
|---|---|
| `bin/miroir` | inverser chaque argument |
| `bin/moyenne` | moyenne de notes entières dans `[0, 20]` |
| `bin/boucle` | trois appels à `fork()` dans une boucle, sans aucune condition |
| `bin/wait` | créer N enfants, attendre chacun et afficher son code de sortie |
| `bin/enfants` | quatre enfants affichant `1..200`, concurremment ou dans l'ordre |
| `bin/analyse-*` | les trois cas d'analyse de code |

```bash
make
./bin/miroir trace soda saper
./bin/moyenne 12 18 3
./bin/boucle
./bin/wait 4
./bin/enfants
./bin/enfants --ordered
```

`boucle` crée un arbre binaire complet de processus : un processus à l'itération
0, deux à l'itération 1, quatre à l'itération 2, soit sept lignes affichées et
huit processus. `enfants --ordered` crée tout de même immédiatement les quatre
enfants, mais les libère un à un via un tuyau, ce qui garantit l'ordre `1..200`
sans sérialiser la création des processus.

L'analyse des trois extraits de code se trouve dans [`analyse/README.md`](analyse/README.md).
