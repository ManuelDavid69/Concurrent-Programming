# TP 10 — Mémoire partagée

| Programme | Objectif |
|---|---|
| `bin/init [capacity] [producers]` | créer les trois segments partagés et leurs sémaphores |
| `bin/clean` | supprimer chaque segment et sémaphore |
| `bin/rdv_n N` | rendez-vous à N voies (le lancer N fois) |
| `bin/prod [items]` | producteur FIFO borné |
| `bin/conso` | consommateur FIFO borné |
| `bin/p0`, `bin/p1`, `bin/p2` | échange de valeurs à trois processus |

```bash
make
./bin/init 5 2
./bin/rdv_n 3 & ./bin/rdv_n 3 & ./bin/rdv_n 3
wait
./bin/prod 20 & ./bin/prod 20 & ./bin/conso & ./bin/conso
wait
./bin/p1 & ./bin/p2 & ./bin/p0
wait
./bin/clean
```

Le rendez-vous à N voies utilise un mutex, un compteur partagé et deux
tourniquets (`RDV_TURNSTILE_0/1`). La parité de génération sélectionne le
tourniquet, de sorte que la même barrière peut être réutilisée plusieurs fois.

Le tampon producteur/consommateur est un tableau circulaire de `capacity`
entiers avec un mutex, un sémaphore `empty` et un sémaphore `full`. `init`
enregistre aussi le nombre attendu de producteurs afin que le dernier puisse
marquer le tampon comme terminé et réveiller tous les consommateurs.

L'échange utilise trois cellules et quatre sémaphores (une arrivée par processus
et une réponse par processus). P1 écrit son maximum dans la cellule 0, P2 écrit
son minimum dans la cellule 1, et P0 les échange (ou écrit `-1` dans la cellule 2
lorsqu'aucun échange n'est nécessaire). P1 et P2 remplacent leur extremum par la
valeur renvoyée par P0 et répètent jusqu'à ce que les tables soient séparées.
