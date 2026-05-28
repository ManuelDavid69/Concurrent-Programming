# TP 11 — Applications de mémoire partagée

## Calculatrice (`bin/client`, `bin/serveur`)

```bash
make
./bin/init
./bin/serveur & ./bin/serveur & ./bin/serveur &
CALC_SEED=42 ./bin/client 5 & CALC_SEED=7 ./bin/client 5
wait
kill -INT %1 %2 %3       # servers stop on SIGINT
./bin/clean
```

Une file de requêtes unique est partagée par tous les clients, et chaque client
dispose d'un emplacement de résultat protégé par son propre sémaphore. Plusieurs
serveurs défilent les requêtes concurremment et écrivent la réponse dans
`results[client_id]` ; le sémaphore de résultat correspondant réveille le bon
client.

## Traitement d'images parallèle (`bin/client_image`, `bin/serveur_image`)

```bash
./bin/init
./bin/serveur_image out --drain & ./bin/serveur_image out --drain &
./bin/client_image out images/*.ppm
wait
```

Les cinq emplacements de tâches sont protégés par les sémaphores `slots`/`items` ;
un serveur prend un chemin, floute l'image PPM et écrit le résultat dans le
répertoire de sortie. Sans `--drain`, les serveurs tournent jusqu'à `SIGINT`,
comme l'exige l'énoncé ; `--drain` est un confort pour les scripts et les tests :
le serveur se termine une fois la file vide et tous les clients terminés.

`init` et `clean` gèrent les ressources des deux parties. Le client d'images doit
utiliser le même répertoire de sortie que les serveurs.
