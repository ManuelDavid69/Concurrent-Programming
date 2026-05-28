# Documentation

Un document par séance de TP. Chacun suit la même structure :

1. objectifs et lien avec le cours ;
2. choix d'implémentation et structure des programmes ;
3. réponses aux questions théoriques ;
4. comment compiler, exécuter et tester les programmes ;
5. résultats observés et, le cas échéant, mesures de performance.

| Séance | Sujet | Document |
|---|---|---|
| 1 | Remise à niveau en C : facteurs premiers, look-and-say, intersection de droites, tri par insertion | [TP01-c-refresher.md](TP01-c-refresher.md) |
| 2 | Tas binaires et tri par tas | [TP02-heap.md](TP02-heap.md) |
| 3 | Flou d'image PPM et digicode à automate | [TP03-image-state-machine.md](TP03-image-state-machine.md) |
| 4 | Arguments de ligne de commande, `fork`, `wait` et arbres de processus | [TP04-arguments-fork.md](TP04-arguments-fork.md) |
| 5 | `fork`/`exec`, parallélisme borné et mini-outil de compilation | [TP05-fork-exec.md](TP05-fork-exec.md) |
| 6 | Tuyaux anonymes, redirections et filtres | [TP06-pipes.md](TP06-pipes.md) |
| 7 | Anneau de processus et calculatrice à base de tuyaux | [TP07-pipes-ring.md](TP07-pipes-ring.md) |
| 8 | Sémaphores System V : précédence, rendez-vous et rendez-vous 1 vers 2 | [TP08-semaphores.md](TP08-semaphores.md) |
| 9 | Signaux : interception, masquage, délai d'expiration et communication père/enfant | [TP09-signals.md](TP09-signals.md) |
| 10 | Mémoire partagée : rendez-vous à N voies, producteur/consommateur et échange de valeurs | [TP10-shared-memory.md](TP10-shared-memory.md) |
| 11 | Calculatrice à mémoire partagée et traitement d'images parallèle | [TP11-shared-memory-applications.md](TP11-shared-memory-applications.md) |
| 12 | Threads POSIX : rendez-vous, producteur/consommateur, échange et Mandelbrot | [TP12-threads.md](TP12-threads.md) |
| 14 | Threads, suite : gestionnaire de ressources et estimation de pi par Monte-Carlo | [TP14-threads-advanced.md](TP14-threads-advanced.md) |
| — | Pipeline graphique logiciel avec threads et barrières | [PIPELINE-graphics.md](PIPELINE-graphics.md) |

## Conventions

- La documentation, les READMEs, les commentaires du code et les messages
  affichés par les programmes sont en français ; les identifiants et les tests
  restent en anglais. Les énoncés du cours sont en français et les réponses
  écrites gardent la terminologie française à l'esprit (*tube*, *sémaphore*,
  *mémoire partagée*, *barrière*).
- Chaque programme est compilé avec `-Wall -Wextra -g` ; la compilation en CI
  utilise en plus `-Werror`.
- Les tests sont des tests d'intégration : ils compilent les binaires, les
  exécutent et vérifient leur sortie, leur code de retour, les fichiers générés
  et, le cas échéant, leurs propriétés de temps ou de synchronisation.
