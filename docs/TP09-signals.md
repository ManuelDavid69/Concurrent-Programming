# TP 9 — Signaux

## Objectifs

- intercepter et ignorer des signaux ;
- arrêter proprement un programme depuis un gestionnaire ;
- envoyer des signaux entre un parent et un enfant ;
- comprendre les groupes de processus et les signaux générés par le terminal ;
- implémenter un délai d'attente (timeout) avec `SIGALRM`.

Un gestionnaire s'exécute de manière asynchrone, entre deux instructions machine du programme principal. Il ne peut appeler que des fonctions **async-signal-safe** et écrire dans des variables `volatile sig_atomic_t`. `write` est sûr ; `printf`, `malloc` et `exit` ne le sont pas (les exemples utilisent `write` et `_exit`).

---

## 1. Interception (`exo1`)

Une boucle infinie affiche un message chaque seconde. Le gestionnaire de `SIGINT` écrit un message et appelle `_exit(0)`.

Lorsque le programme est lancé en arrière-plan avec `./exo1 &`, `Ctrl+C` ne l'atteint pas : le terminal envoie `SIGINT` au **groupe de processus au premier plan**, et la tâche en arrière-plan n'en fait pas partie. Utilisez plutôt `kill -INT <pid>`.

## 2. Ignorer un signal (`exo2`)

`signal(SIGINT, SIG_IGN)` rend `Ctrl+C` inopérant. `SIGKILL` ne peut être ni intercepté, ni bloqué, ni ignoré ; c'est donc le seul moyen fiable d'arrêter le programme.

## 3. Arrêt propre (`exo3`)

Le gestionnaire positionne un drapeau `volatile sig_atomic_t`. La boucle est conditionnée par ce drapeau, de sorte que le programme quitte la boucle normalement, vide sa sortie et revient de `main`. C'est le modèle recommandé lorsque le programme a des ressources à libérer.

## 4. Le parent tue son enfant (`exo4`)

Le père affiche trois fois, puis envoie `SIGKILL` à son enfant et attend. L'enfant ne peut pas intercepter `SIGKILL` ; il est terminé par le noyau. Le parent appelle tout de même `waitpid`, qui récupère le statut de terminaison et évite un zombie.

## 5. `Ctrl+C` et la hiérarchie (`exo5`)

L'enfant installe un gestionnaire de `SIGINT`, mais pas le parent. Lorsque `Ctrl+C` est enfoncé, le terminal envoie `SIGINT` à tout le groupe de processus au premier plan :

- le parent a l'action par défaut → il se termine ;
- l'enfant intercepte le signal, affiche un message et continue.

L'enfant survit à son parent. Cela montre qu'un signal est délivré à chaque processus du groupe, et qu'un gestionnaire ne change l'action par défaut que pour le processus qui l'a installé.

## 6. `SIGUSR1` / `SIGUSR2` (`exo6`)

L'enfant bloque `SIGUSR1` et `SIGUSR2`, installe un gestionnaire unique, signale qu'il est prêt via un tube, et attend avec `sigsuspend`. Le parent :

- envoie `SIGUSR1` aux itérations 3 et 5 ;
- envoie `SIGUSR2` après la boucle pour annoncer la fin.

Un gestionnaire unique reçoit le numéro du signal et effectue le dispatch :

```c
if (signal_number == SIGUSR1) { /* display */ }
else if (signal_number == SIGUSR2) { running = 0; }
```

Bloquer les signaux et utiliser `sigsuspend` supprime la course entre l'arrivée d'un signal et l'appel à `pause`.

## 7. Délai d'attente (`exo7`)

Le programme arme `alarm(5)` (ou `PC_TIMEOUT_SECONDS`), puis lit une ligne avec `fgets`. Le gestionnaire de `SIGALRM` ne fait que positionner un drapeau. Comme `sigaction` est utilisé sans `SA_RESTART`, le `fgets` interrompu renvoie la main et le programme remarque le drapeau :

- entier valide → `alarm(0)`, affiche `Ok merci !!`, sortie 0 ;
- ligne invalide → redemande ;
- délai dépassé → affiche `Trop tard !!`, sortie 1.

La lecture est ligne par ligne (`fgets` + analyse stricte), donc une entrée non entière ne fait pas planter le programme.

---

## Compilation, exécution et tests

```bash
make -C tp09
./tp09/bin/exo1
./tp09/bin/exo2
./tp09/bin/exo3
./tp09/bin/exo4
./tp09/bin/exo5
./tp09/bin/exo6
./tp09/bin/exo7
python3 -m pytest tests/test_tp09.py -q
```

Les tests envoient de vrais signaux (`SIGINT`, `SIGKILL`, `SIGUSR1`, `SIGUSR2`), vérifient les sorties propres, s'assurent que `exo2` survit à `SIGINT`, que l'enfant de `exo5` survit à son parent, et que `exo7` accepte un entier valide ou signale le délai dépassé.
