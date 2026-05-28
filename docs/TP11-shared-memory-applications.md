# TP 11 — Applications de la mémoire partagée

## Objectifs

- construire une calculatrice multi-serveurs avec mémoire partagée et sémaphores ;
- prendre en charge plusieurs clients envoyant des requêtes simultanément ;
- renvoyer chaque résultat au bon client sans recopier toute la requête ;
- traiter un lot d'images en parallèle avec un pool de processus workers.

---

## 1. Calculatrice à mémoire partagée

### Organisation des données

```c
typedef struct {
    int client_id;
    int left;
    int right;
    char operator;
} CalcRequest;

typedef struct {
    int left;
    int right;
    int result;
    char operator;
} CalcResponse;

typedef struct {
    CalcRequest queue[16];
    CalcResponse results[8];
    int head, tail, count;
    int client_pid[8];
} CalcShared;
```

Une file de requêtes est partagée par tous les clients. Chaque client possède également un emplacement de résultat et un sémaphore privé (`CALC_RESULT_BASE + slot`).

### Sémaphores

| Sémaphore | Initial | Rôle |
|---|---:|---|
| `CALC_MUTEX` | 1 | protège la file, les compteurs et les résultats |
| `CALC_SLOTS` | 16 | places libres dans la file de requêtes |
| `CALC_ITEMS` | 0 | requêtes en attente de traitement |
| `CALC_RESULT_BASE + i` | 0 | résultat prêt pour l'emplacement client `i` |

### Client

1. acquiert un emplacement libre sous le mutex (`client_pid[i] == 0`) ;
2. répète `requests` fois :
   - attend un emplacement libre dans la file, ajoute la requête, fait un post sur `items` ;
   - attend sur son sémaphore de résultat ;
   - lit `results[slot]` sous le mutex et l'affiche ;
3. libère l'emplacement.

Le client envoie une requête et attend son résultat avant d'envoyer la suivante, comme demandé par l'énoncé.

### Serveur

Le serveur boucle :

1. attend `items` ;
2. retire de la file sous le mutex, fait un post sur `slots` ;
3. évalue, écrit `results[client_id]` sous le mutex, fait un post sur le sémaphore de résultat du client.

Plusieurs serveurs peuvent fonctionner en même temps ; la file est la seule structure partagée producteur/consommateur, et le mutex rend le retrait atomique. Un gestionnaire de `SIGINT` positionne un drapeau ; le serveur utilise `sem_p_interruptible` pour que l'attente bloquante revienne avec `EINTR` et que la boucle puisse sortir proprement.

### Pourquoi un identifiant de client ?

Le résultat pourrait être envoyé via un tube, mais ici le serveur a besoin d'un moyen rapide de trouver la zone de résultat du client. L'indice d'emplacement (`0..7`) est cette clé. Il reste stable même si le pid est réutilisé par le système d'exploitation, et il est plus petit qu'un pid.

---

## 2. Traitement d'images en parallèle

### Organisation des données

```c
typedef struct {
    int head, tail, count;
    int active_clients;
    int finished;
    char tasks[5][10000];
} ImageShared;
```

Cinq emplacements de tâches, chacun pouvant contenir un chemin de 10 000 caractères maximum. Un chemin ne peut pas être transmis sous forme de pointeur : le client et les serveurs sont des processus différents, donc les caractères doivent être copiés dans le segment partagé.

### Protocole

- `IMG_SLOTS` (initial 5) et `IMG_ITEMS` (initial 0) implémentent la file bornée ;
- le client copie chaque chemin dans `tasks[tail]`, avance `tail`, fait un post sur `items` ;
- un serveur attend un élément, copie `tasks[head]` dans un tampon local, avance `head`, fait un post sur `slots`, puis traite le fichier ;
- le client incrémente `active_clients` au démarrage et le décrémente lorsqu'il a tout mis en file ; le dernier client positionne `finished`.

Les serveurs en mode normal tournent jusqu'à `SIGINT`, comme demandé. `--drain` est une commodité pour les scripts et les tests : le serveur interroge la file et sort une fois que `finished` est positionné et que la file est vide. Un serveur qui a déjà retiré une tâche la termine avant de sortir.

### Traitement

Le serveur charge le fichier PPM avec le même `image_load` que celui utilisé au TP 3, applique le flou par noyau 3×3 (kernel-crop) et enregistre le résultat en P6 dans le répertoire de sortie. Il affiche les chemins source et destination.

### Résultats

Avec cinq images 4×4 et deux serveurs, les cinq sorties sont produites. Les serveurs peuvent répartir le travail de manière inégale (un seul peut traiter les cinq s'il est ordonnancé en premier) ; l'exigence est que chaque fichier soit traité exactement une fois et que le répertoire de sortie contienne exactement les fichiers attendus.

---

## Compilation, exécution et tests

```bash
make -C tp11
./tp11/bin/init
./tp11/bin/serveur & ./tp11/bin/serveur &
CALC_SEED=42 ./tp11/bin/client 5
kill -INT %1 %2
./tp11/bin/serveur_image out --drain & ./tp11/bin/serveur_image out --drain &
./tp11/bin/client_image out images/*.ppm
wait
./tp11/bin/clean
python3 -m pytest tests/test_tp11.py -q
```

Les tests démarrent deux serveurs de calculatrice, lancent deux clients en parallèle, vérifient l'arithmétique de chaque réponse et l'arrêt propre sur `SIGINT`, puis traitent cinq images PPM générées avec deux serveurs en mode drain et contrôlent chaque sortie.
