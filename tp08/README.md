# TP 8 — Sémaphores System V

| Programme | Objectif |
|---|---|
| `bin/init` | créer l'ensemble de sémaphores (neuf sémaphores) et l'état partagé |
| `bin/clean` | supprimer les deux ressources |
| `bin/first`, `bin/second` | précédence stricte entre deux processus indépendants |
| `bin/rdv2` | rendez-vous pour deux processus (`rdv2 0` et `rdv2 1`) |
| `bin/rdv3` | rendez-vous pour trois processus (`rdv3 0`, `1`, `2`) |
| `bin/emetteur`, `bin/recepteur` | un émetteur attend deux récepteurs |

```bash
make
./bin/init
./bin/first & ./bin/second
wait
./bin/rdv2 0 & ./bin/rdv2 1
wait
./bin/rdv3 0 & ./bin/rdv3 1 & ./bin/rdv3 2
wait
./bin/clean
```

L'ensemble de sémaphores contient :

| Indice | Sémaphore | Valeur initiale | Rôle |
|---:|---|---:|---|
| 0 | `SEM_FIRST` | 0 | `second` attend `first` |
| 1–2 | `SEM_RDV2_*` | 0 | rendez-vous à deux processus |
| 3–5 | `SEM_RDV3_*` | 0 | rendez-vous à trois processus |
| 6 | `SEM_MUTEX` | 1 | protège les compteurs partagés |
| 7 | `SEM_EMETTEUR` | 0 | émetteurs bloqués |
| 8 | `SEM_RECEPTEUR` | 0 | récepteurs bloqués |

Le segment partagé ne stocke que le nombre d'émetteurs en attente, le nombre de
récepteurs en attente et un compteur de messages. `init` supprime toute ressource
résiduelle avant de les créer, afin qu'un test planté ne pollue pas l'exécution
suivante.

`rdv2` et `rdv3` utilisent un sémaphore par participant : chaque processus poste
les sémaphores des autres, puis attend le sien. Cela se généralise à N
participants tant que N est connu à l'avance ; la barrière du TP 10 lève cette
limitation grâce à un compteur partagé.
