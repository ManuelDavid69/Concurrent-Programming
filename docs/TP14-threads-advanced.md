# TP 14 — Threads, suite

## Objectifs

- partager une ressource limitée entre N threads workers ;
- implémenter une paire bloquante `demander(k)` / `rendre(k)` avec une variable de condition ;
- surveiller l'invariant partagé depuis un thread contrôleur ;
- estimer pi avec la méthode de Monte-Carlo, d'abord en mono-thread puis en multi-thread, et comparer les temps d'exécution.

---

## 1. Gestionnaire de billes (`billes`)

### État partagé

```c
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int available;
    int maximum;
    int workers_done;
    int stop_controller;
    int controller_checks;
    int invariant_error;
} BallManager;
```

### Protocole des workers

Chaque worker répète `iterations` fois :

```
demander(k)
utilise les k balles (sommeil aléatoire)
rendre(k)
```

`demander(k)` :

```c
lock(mutex);
while (available < k)
    wait(condition, mutex);
available -= k;
unlock(mutex);
```

`rendre(k)` :

```c
lock(mutex);
available += k;
broadcast(condition);
unlock(mutex);
```

Le `while` est essentiel : une diffusion signale seulement que l'état a changé, pas qu'assez de billes sont désormais disponibles. Plusieurs workers peuvent se réveiller en même temps et un seul peut trouver un nombre suffisant de billes ; les autres doivent se rendormir.

Le scénario par défaut est `N = 4`, `maximum = 9` et des demandes `4, 3, 5, 2`. Comme `4 + 3 + 5 + 2 = 14 > 9`, les workers ne peuvent pas tous être satisfaits en même temps, ce qui exerce le chemin d'attente.

### Contrôleur

Le thread contrôleur vérifie l'invariant

```
0 <= available <= maximum
```

une fois par seconde (mis à l'échelle par `PC_SLEEP_SCALE` dans les tests), incrémente son compteur de vérifications et enregistre une violation si l'invariant est rompu. Le thread principal positionne `stop_controller` après avoir joint les workers, puis joint le contrôleur.

Le mutex protège à la fois `available` et les drapeaux du contrôleur ; le worker affiche le nombre de billes restantes après avoir pris un instantané cohérent sous le mutex.

---

## 2. Estimation de pi par Monte-Carlo (`pi`)

### Méthode

Un point aléatoire `(x, y)` est tiré uniformément dans `[0, 1] × [0, 1]`. Il se trouve dans le quart du cercle unité lorsque `x² + y² <= 1`. L'aire du quart de cercle est `pi / 4`, donc

```
pi ≈ 4 * hits / samples
```

L'erreur type diminue comme `1 / sqrt(N)` : un million d'échantillons donnent typiquement deux ou trois décimales correctes. Le but est la parallélisation, pas une précision record.

### Générateur aléatoire

Chaque thread possède son propre état `xorshift64*`. Cela évite un `rand()` partagé (qui nécessiterait un verrou et sérialiserait la boucle chaude) et donne des flux indépendants. Les graines sont dérivées du temps, du pid et de l'indice du thread.

### Parallélisation

- `samples` est réparti aussi équitablement que possible entre les threads (les `samples % threads` premiers threads reçoivent un point de plus) ;
- chaque thread compte ses propres succès ;
- le thread principal additionne les succès après les jointures.

Aucun mutex n'est nécessaire dans la boucle chaude : chaque thread n'écrit que son propre champ `hits`, et le thread principal le lit après `pthread_join`.

`--compare` exécute la version mono-thread, puis la version multi-thread, et affiche les deux estimations ainsi que l'accélération mesurée. Sur une machine à plusieurs cœurs, la version multi-thread est typiquement 2 à 4 fois plus rapide pour un grand N ; la valeur exacte dépend de l'ordonnanceur et du nombre de cœurs disponibles.

---

## Compilation, exécution et tests

```bash
make -C tp14
./tp14/bin/billes 4 9 3 4,3,5,2
./tp14/bin/pi 1000000 4
./tp14/bin/pi --compare 1000000 4
python3 -m pytest tests/test_tp14.py -q
```

Les tests exécutent le scénario de billes par défaut, vérifient que l'invariant n'est jamais violé, que toute demande impossible est rejetée, et que les deux estimations de pi sont à moins de 0,05 de la valeur réelle.
