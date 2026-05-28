# TP 10 — Mémoire partagée

## Objectifs

- créer et attacher des segments de mémoire partagée System V ;
- construire une barrière réutilisable à N participants à partir d'un mutex, d'un compteur et de deux tourniquets ;
- implémenter un producteur/consommateur FIFO borné avec plusieurs producteurs et consommateurs ;
- implémenter l'algorithme d'échange de valeurs à trois processus (P0, P1, P2).

---

## 1. Ressources

`init [capacity] [producers]` crée trois groupes indépendants :

| Groupe | Segment partagé | Sémaphores |
|---|---|---|
| rendez-vous | `RendezVous { count, generation }` | mutex, tourniquet 0, tourniquet 1 |
| producteur/consommateur | `Buffer` de `capacity` entiers | mutex, empty, full |
| échange | `Echange { first, second, third }` | P1, P2, P0 vers P1, P0 vers P2 |

`clean` supprime les six ressources. Supprimer les ressources obsolètes au début de `init` rend les programmes réexécutables après un plantage.

---

## 2. Rendez-vous à N participants

Chaque processus appelle `rdv_n N`, tous avec le même N. La barrière est :

```
lock(mutex)
generation = state->generation
state->count++
if (state->count == N) {
    state->count = 0
    state->generation = generation + 1
    unlock(mutex)
    post(turnstile[generation % 2]) N - 1 times
} else {
    unlock(mutex)
    wait(turnstile[generation % 2])
}
```

- le **dernier** processus remet le compteur à zéro et libère les `N-1` attenteurs ;
- la **parité de génération** sélectionne l'un des deux tourniquets, ce qui permet de réutiliser la barrière : un processus qui arrive pour le tour suivant attend sur l'autre tourniquet au lieu de consommer un jeton du tour précédent ;
- le mutex protège `count` et `generation`.

La barrière est réutilisable et utilise un nombre constant de sémaphores, indépendamment de N. La seule condition est que chaque participant utilise le même N. Si un processus manque, les autres se bloquent indéfiniment, ce qui correspond à la sémantique attendue d'un rendez-vous.

**Réponse à la question de l'énoncé.** Avec les sémaphores vus jusqu'ici, un rendez-vous pour N processus dont N **n'est pas connu au moment de la création des sémaphores** ne peut pas être implémenté avec un ensemble fixe de sémaphores par processus. La barrière ci-dessus le résout parce que N est passé à l'exécution et le compteur partagé enregistre le nombre d'arrivées. Si N était totalement inconnu, il faudrait une ressource dynamique (un compteur partagé et un mécanisme de type condition), ou un processus gestionnaire qui compte les arrivées. Un pur sémaphore ne peut pas découvrir N par lui-même.

---

## 3. Producteur/consommateur borné

Le tampon est un tableau circulaire :

```
items[tail] = value; tail = (tail + 1) % capacity; count++
value = items[head]; head = (head + 1) % capacity; count--
```

- `empty` est initialisé à `capacity`, `full` à 0 ;
- un producteur attend sur `empty` avant d'écrire, puis fait un post sur `full` ;
- un consommateur attend sur `full` avant de lire, puis fait un post sur `empty` ;
- le mutex protège `head`, `tail` et `count`.

L'ordre FIFO découle des indices circulaires : le producteur écrit à `tail`, le consommateur lit à `head`, et tous deux avancent dans la même direction.

`init` enregistre le nombre attendu de producteurs. Chaque producteur s'enregistre au démarrage (`producers_started++`, `producers_active++`). Lorsque le dernier producteur termine, il positionne `finished` et fait `full` posts afin que chaque consommateur bloqué sur un tampon vide se réveille, constate le drapeau et sorte. Cela évite le problème classique d'un consommateur qui attend indéfiniment après le dernier élément.

Plusieurs producteurs et consommateurs sont pris en charge tant que le nombre de producteurs donné à `init` correspond au nombre réellement lancé.

---

## 4. Échange de valeurs (P0, P1, P2)

P1 et P2 détiennent chacun un tableau de huit entiers. À chaque tour :

1. P1 écrit son **maximum** dans la cellule 0 et fait un post sur `P1` ;
2. P2 écrit son **minimum** dans la cellule 1 et fait un post sur `P2` ;
3. P0 attend les deux, puis :
   - si `cell0 > cell1`, échange les deux valeurs et met la cellule 2 à 0 ;
   - sinon met la cellule 2 à `-1` ;
4. P0 fait un post sur un sémaphore de réponse par thread (`P0 vers P1` et `P0 vers P2`) ;
5. P1 remplace son maximum par la valeur de la cellule 0 ; P2 remplace son minimum par la valeur de la cellule 1 ; les deux bouclent.

Lorsque `cell2 == -1`, aucun échange n'est nécessaire : toutes les valeurs de P1 sont inférieures à toutes les valeurs de P2. P1 et P2 affichent leurs tableaux finaux et sortent.

Les tableaux initiaux sont `{20, 30, ..., 90}` pour P1 et `{1, 2, ..., 8}` pour P2, donc huit échanges sont nécessaires pour les séparer. Les tableaux finaux sont `{1..8}` et `{20..90}`.

Les trois cellules sont protégées par le protocole lui-même : P0 ne les lit qu'après les deux arrivées, et P1/P2 ne lisent leur cellule qu'après leur propre sémaphore de réponse. Deux sémaphores de réponse sont nécessaires : avec un seul sémaphore compteur posté deux fois, un processus pourrait consommer les deux jetons et lire une cellule qu'il a déjà écrasée.

---

## Compilation, exécution et tests

```bash
make -C tp10
./tp10/bin/init 5 2
./tp10/bin/rdv_n 3 & ./tp10/bin/rdv_n 3 & ./tp10/bin/rdv_n 3
wait
./tp10/bin/prod 20 & ./tp10/bin/prod 20 & ./tp10/bin/conso & ./tp10/bin/conso
wait
./tp10/bin/p1 & ./tp10/bin/p2 & ./tp10/bin/p0
wait
./tp10/bin/clean
python3 -m pytest tests/test_tp10.py -q
```

Les tests couvrent N = 1, 2, 3 et 5 pour la barrière, l'ordre FIFO avec un producteur, plusieurs producteurs et consommateurs, et la séparation finale des deux tableaux par l'échange.
