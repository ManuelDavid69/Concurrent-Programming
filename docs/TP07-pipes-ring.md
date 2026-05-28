# TP 7 — Tubes, suite

## Objectifs

- organiser N processus en un anneau de tubes anonymes ;
- faire circuler un message structuré dans l'anneau ;
- concevoir un petit serveur requête/réponse avec un tube par client ;
- garder claire la propriété des descripteurs dans chaque processus.

---

## 1. Anneau de processus (`greater`)

### Topologie

`pipe[i]` relie le processus `i` au processus `(i + 1) % N`. Le processus `i` :

- lit depuis `pipe[(i - 1 + N) % N]` ;
- écrit dans `pipe[i]`.

```
process 0 -> pipe 0 -> process 1 -> pipe 1 -> ... -> process N-1 -> pipe N-1 -> process 0
```

Chaque processus ferme toutes les extrémités de tube sauf son extrémité de lecture entrante et son extrémité d'écriture sortante. C'est ce qui rend l'anneau bien défini : aucun processus ne garde un descripteur qui pourrait empêcher l'EOF ou créer un interblocage.

### Protocole

Chaque processus tire une valeur et construit un `Candidate` :

```c
typedef struct {
    int value;
    pid_t pid;
    int rank;
} Candidate;
```

Le processus 0 envoie son candidat au processus 1. Chaque autre processus lit le candidat entrant, conserve le plus grand des deux (le sien et celui qui arrive) et transmet le gagnant. Après un tour complet, le processus 0 reçoit le maximum global avec le pid et le rang du processus qui l'a produit.

Le programme affiche chaque valeur générée puis :

```
Le plus grand nombre = 2602 - pid = 4235 - Numéro 5
```

L'ordre des lignes `processus ...` est arbitraire ; la ligne finale est affichée une fois l'anneau entièrement parcouru.

Avec `N = 1`, le processus 0 écrit dans son propre tube et y lit ; le résultat est son propre candidat. La suite de tests exerce N = 1, 2, 3, 6 et 12.

---

## 2. Calculatrice (`calculator`)

### Topologie

```
client 0 --\
client 1 ----> request pipe ----> server
client 2 --/                       |
   ^                               |
   +---- response pipe per client -+
```

Il y a **un** tube de requêtes pour tous les clients et **un tube de réponse par client**. Le serveur crée les tubes avant de créer les clients par fork, de sorte que chaque client hérite des descripteurs.

### Requête et réponse

```c
typedef struct {
    int client_id;
    int left;
    int right;
    char operator;
} Request;

typedef struct {
    int left;
    int right;
    int result;
    char operator;
} Response;
```

L'identifiant de client est un indice entre 0 et N-1, pas un pid. Le serveur l'utilise directement comme indice dans le tableau des tubes de réponse, ce qui est plus simple et évite une recherche.

Chaque client envoie exactement une requête et attend sa réponse. Le serveur lit les requêtes dans l'ordre d'arrivée et répond immédiatement, de sorte que plusieurs clients peuvent être actifs en même temps. Les structures `Request`/`Response` sont assez petites pour être écrites atomiquement dans un tube (`PIPE_BUF`), mais le code utilise tout de même `write_all`/`read_all` par robustesse.

Opérateurs pris en charge : `+`, `-`, `*`, `/`. Une division par zéro est rejetée par le serveur (le résultat est mis à 0 et une erreur est affichée), bien que les clients ne génèrent jamais de diviseur nul.

`CALC_SEED` rend les requêtes des clients reproductibles : le client `i` utilise `seed + i * 7919`, de sorte que les clients ne génèrent pas tous la même requête.

---

## Compilation, exécution et tests

```bash
make -C tp07
./tp07/bin/greater 6
CALC_SEED=42 ./tp07/bin/calculator 4
python3 -m pytest tests/test_tp07.py -q
```

Les tests vérifient que l'anneau renvoie le vrai maximum pour plusieurs tailles, y compris avec un seul processus, et que chaque réponse de la calculatrice est arithmétiquement correcte et reproductible avec une graine fixée.
