# TP 9 — Signaux

| Programme | Objectif |
|---|---|
| `bin/exo1` | interrompre une boucle infinie avec `SIGINT` |
| `bin/exo2` | ignorer `SIGINT` ; l'arrêter avec `SIGKILL` |
| `bin/exo3` | arrêt propre via un drapeau `volatile sig_atomic_t` |
| `bin/exo4` | le père envoie `SIGKILL` à son enfant après trois itérations |
| `bin/exo5` | l'enfant intercepte `SIGINT` pendant que le père se termine |
| `bin/exo6` | communication `SIGUSR1`/`SIGUSR2` entre père et enfant |
| `bin/exo7` | lire un entier avant un délai d'expiration de cinq secondes via `SIGALRM` |

```bash
make
./bin/exo1          # Ctrl+C
./bin/exo2          # Ctrl+C ne fait rien ; utiliser kill -KILL <pid>
./bin/exo3          # Ctrl+C effectue un arrêt propre
./bin/exo4
./bin/exo5          # Ctrl+C : le fils survit
./bin/exo6
./bin/exo7
```

Points importants :

- un drapeau `volatile sig_atomic_t` est le seul état qu'un gestionnaire doit
  écrire ;
- `write` est sûr en contexte de signal asynchrone, `printf` ne l'est pas ;
- `sa_flags = 0` évite `SA_RESTART`, ce qui permet à `fgets` de revenir et au
  délai d'expiration d'être remarqué ;
- `SIGKILL` et `SIGSTOP` ne peuvent être ni interceptés ni ignorés ;
- les signaux sont délivrés au groupe de processus de premier plan, donc `Ctrl+C`
  atteint à la fois le père et l'enfant dans `exo5` ; le père meurt, l'enfant
  capture le signal et continue.
