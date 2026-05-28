# TP 7 — Tuyaux, suite

## Anneau de processus (`bin/greater`)

```bash
make
./bin/greater 6
```

Chacun des N processus tire une valeur aléatoire, l'affiche et transmet à son
successeur la plus grande valeur candidate vue jusqu'ici. Le processus 0 démarre
l'anneau et reçoit la valeur candidate finale après un tour complet.

L'anneau utilise N tuyaux : le tuyau `i` relie le processus `i` au processus
`(i + 1) % N`. Chaque processus ferme tous les descripteurs sauf ses tuyaux
entrant et sortant. La dernière ligne est

```
Le plus grand nombre = 2602 - pid = 4235 - Numéro 5
```

## Calculatrice (`bin/calculator`)

```bash
./bin/calculator 4
CALC_SEED=42 ./bin/calculator 6
```

Le serveur crée un tuyau de requêtes (clients → serveur) et un tuyau de réponses
par client (serveur → client). Chaque client envoie une `Request` contenant son
identifiant, deux opérandes et un opérateur parmi `+ - * /`, puis attend sa
`Response`. Le serveur évalue la requête et écrit le résultat dans le tuyau de
réponses sélectionné par l'identifiant du client.

L'identifiant du client n'est pas le pid : un indice attribué par le serveur
suffit et rend la recherche du tuyau de réponses immédiate. Toutes les extrémités
inutilisées sont fermées chez le serveur comme chez les clients.
