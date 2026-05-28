# TP 8 — Sémaphores System V

## Objectifs

- créer, initialiser et supprimer un ensemble de sémaphores System V ;
- utiliser les opérations P (wait) et V (post) pour imposer une précédence ;
- implémenter un rendez-vous pour deux et trois processus ;
- implémenter un rendez-vous 1 émetteur / 2 récepteurs avec des compteurs partagés ;
- discuter de la généralisation d'un rendez-vous à N processus.

---

## 1. L'ensemble de sémaphores

`init` supprime toute ressource obsolète, puis crée un ensemble de neuf sémaphores et un segment de mémoire partagée.

| Indice | Sémaphore | Initial | Rôle |
|---:|---|---:|---|
| 0 | `SEM_FIRST` | 0 | `second` attend `first` |
| 1 | `SEM_RDV2_0` | 0 | rendez-vous 2, processus 0 |
| 2 | `SEM_RDV2_1` | 0 | rendez-vous 2, processus 1 |
| 3 | `SEM_RDV3_0` | 0 | rendez-vous 3, processus 0 |
| 4 | `SEM_RDV3_1` | 0 | rendez-vous 3, processus 1 |
| 5 | `SEM_RDV3_2` | 0 | rendez-vous 3, processus 2 |
| 6 | `SEM_MUTEX` | 1 | protège les compteurs partagés |
| 7 | `SEM_EMETTEUR` | 0 | émetteurs en attente de deux récepteurs |
| 8 | `SEM_RECEPTEUR` | 0 | récepteurs en attente d'un rendez-vous |

`clean` supprime les deux ressources. Lancer `init` avant chaque test garantit un état connu, même après un plantage.

Le segment partagé contient :

```c
typedef struct {
    int emetteurs_attente;
    int recepteurs_attente;
    int messages;
} Etat;
```

Les compteurs sont protégés par `SEM_MUTEX` ; les sémaphores servent au blocage, pas à l'attente active.

> **Pourquoi pas de `SEM_UNDO` ?** `SEM_UNDO` est utile pour un verrou détenu par un processus susceptible de planter. Pour un sémaphore utilisé comme **signal**, il est dangereux : si un processus fait un post puis se termine avant que l'attenteur ne consomme le jeton, le noyau annule le post. Ici, les programmes sont de courte durée et les ressources sont réinitialisées par `init`, donc les wrappers utilisent un simple `semop` sans `SEM_UNDO`.

---

## 2. Précédence (`first`, `second`)

`first` affiche, dort entre 3 et 5 secondes (mis à l'échelle dans les tests), affiche de nouveau, puis fait un post sur `SEM_FIRST`. `second` affiche, attend `SEM_FIRST`, affiche que `first` a terminé, dort de 1 à 4 secondes, et affiche son message final.

Il n'y a pas de relation parent/enfant, et l'ordre de lancement n'a pas d'importance :

- si `second` démarre en premier, il se bloque sur le sémaphore ;
- si `first` démarre en premier, il fait un post avant que `second` n'atteigne l'attente, et le sémaphore conserve le jeton.

Le sémaphore agit donc comme un événement ponctuel, et non comme un verrou.

---

## 3. Rendez-vous pour deux et trois processus

Pour N participants, l'implémentation crée un sémaphore par participant. Le participant `i` :

1. simule un peu de travail ;
2. fait un post sur le sémaphore de chacun des autres participants ;
3. attend sur son propre sémaphore.

Pour N = 2 :

```
P0: post(S1), wait(S0)
P1: post(S0), wait(S1)
```

Pour N = 3, chaque processus fait un post sur les deux autres sémaphores et attend sur le sien. Cela fonctionne car chaque post est un jeton qui subsiste jusqu'à ce que la cible attende, et aucun processus ne peut franchir sa propre attente avant que tous les autres aient fait un post.

Le modèle exige que N soit connu au moment de la création des sémaphores. Il utilise aussi `N` sémaphores et `N` posts par processus, soit `O(N^2)` posts au total. Le TP 10 le remplace par une barrière fondée sur un compteur partagé, un mutex et deux tourniquets, qui fonctionne pour tout N fourni à l'exécution et utilise `O(1)` posts par processus.

---

## 4. Rendez-vous 1 émetteur / 2 récepteurs

L'émetteur doit attendre que **deux** récepteurs soient présents. Un récepteur doit attendre qu'il y ait un émetteur **et** un autre récepteur. L'état est :

```c
int emetteurs_attente;
int recepteurs_attente;
```

### Émetteur

```
lock(mutex)
if (recepteurs_attente >= 2) {
    recepteurs_attente -= 2;
    immediate = true;
} else {
    emetteurs_attente++;
    immediate = false;
}
unlock(mutex)

if (immediate)
    post(recepteur); post(recepteur);
else
    wait(emetteur);
```

### Récepteur

```
lock(mutex)
if (emetteurs_attente >= 1 && recepteurs_attente >= 1) {
    emetteurs_attente--;
    recepteurs_attente--;
    immediate = true;
} else {
    recepteurs_attente++;
    immediate = false;
}
unlock(mutex)

if (immediate)
    post(emetteur); post(recepteur);
else
    wait(recepteur);
```

Le second récepteur est celui qui complète le trio : il consomme l'émetteur en attente et le récepteur en attente, puis réveille les deux. Si l'émetteur arrive en dernier et trouve deux récepteurs, il réveille les deux directement.

Deux sémaphores suffisent car les compteurs, protégés par le mutex, enregistrent exactement combien de processus sont bloqués ; les sémaphores n'assurent que l'attente passive et le réveil.

---

## Compilation, exécution et tests

```bash
make -C tp08
./tp08/bin/init
./tp08/bin/first & ./tp08/bin/second
wait
./tp08/bin/rdv2 0 & ./tp08/bin/rdv2 1
wait
./tp08/bin/rdv3 0 & ./tp08/bin/rdv3 1 & ./tp08/bin/rdv3 2
wait
./tp08/bin/recepteur & ./tp08/bin/emetteur & ./tp08/bin/recepteur
wait
./tp08/bin/clean
python3 -m pytest tests/test_tp08.py -q
```

Les tests vérifient que `second` est réellement bloqué avant que `first` ne fasse son post, que les rendez-vous à deux et trois processus aboutissent, et que l'émetteur est libéré à la fois lorsqu'il arrive avant les récepteurs et lorsqu'il arrive après eux.
