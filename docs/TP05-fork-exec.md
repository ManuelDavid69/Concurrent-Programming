# TP 5 — `fork`, `exec` et parallélisme borné

## Objectifs

- remplacer le programme courant par un autre à l'aide de `fork` + `execvp` ;
- comprendre la différence entre `&` (parallèle) et `;` (séquentiel) ;
- maintenir un nombre borné de processus enfants actifs ;
- construire un `make` miniature avec compilation parallèle et édition de liens conditionnelle.

---

## 1. Commandes simultanées et successives

Une commande du shell est exécutée en créant un processus par fork et en appelant `execvp` dans l'enfant. Le parent attend l'enfant avec `waitpid`.

### `simultane` — `du -sh /usr/share & ls -l & ps`

Les trois enfants sont créés avant le premier `waitpid`. Le parent affiche `Lancé` pour chaque pid, puis attend dans un ordre fixe et affiche `Terminé`. Les trois commandes se chevauchent réellement.

`SIMULTANE_PATH` remplace `/usr/share` dans la commande `du` ; la valeur par défaut est le chemin de l'énoncé. Les tests utilisent un petit répertoire temporaire afin de ne pas dépendre de la taille de `/usr/share` sur la machine.

### `successif` — `du -sh /usr/share ; ls -l ; ps`

La boucle crée un enfant, l'attend, affiche le résultat, puis seulement démarre la commande suivante. La sortie est strictement `Lancé`, `Terminé`, `Lancé`, ....

`execvp` parcourt le `PATH`, ce qui évite de coder en dur `/usr/bin/du` et consorts. Si `execvp` échoue, l'enfant affiche l'erreur et se termine avec un statut non nul ; le parent signale `Échec`.

---

## 2. Vignettes avec au plus huit conversions

`miniature` parcourt le répertoire source avec `opendir`/`readdir`, ne conserve que les noms d'images réguliers (`.jpg`, `.png`, `.ppm`, `.bmp`, `.gif`, `.tif`, `.webp`), et construit les chemins source et destination.

Pour chaque image, il crée un enfant qui exécute :

```
convert source -resize 10% destination
```

Au plus **huit** enfants sont actifs. Lorsque le pool est plein :

```c
while (active >= MAX_CONVERSIONS) {
    report_finished(running, MAX_CONVERSIONS, &active, &failed);
}
```

`report_finished` appelle `waitpid(-1, ...)`, retrouve l'emplacement correspondant par pid, affiche le nom du fichier et met à jour le nombre d'actifs. L'emplacement est ensuite réutilisé par la tâche suivante. À la fin, les enfants restants sont drainés de la même manière.

Le répertoire de destination est créé si nécessaire. Un `convert` qui échoue produit `Échec conversion: file (code ...)` et le programme renvoie 1, mais les autres conversions sont tout de même attendues.

---

## 3. Téléchargements parallèles avec au plus quatre `wget`

Le même schéma de pool est utilisé avec `MAX_DOWNLOADS = 4`. Le dernier argument est le répertoire de destination ; chaque argument précédent est une URL. Chaque enfant exécute :

```
wget -q -P destination URL
```

`-q` garde la sortie lisible, `-P` sélectionne le répertoire de destination. Le parent affiche `Début téléchargement` et `Téléchargement terminé` avec l'URL et le pid. Le pool borné évite qu'une longue liste d'URL ne crée un nombre illimité de processus.

---

## 4. `make` miniature

`make.c` prend un ou plusieurs fichiers `.c` suivis du nom de l'exécutable :

```
./bin/make main.c mathutils.c message.c program
```

1. Pour chaque source, un enfant exécute `gcc -Wall -Wextra -g -c source -o basename.o`.
2. Le parent attend chaque compilation et affiche un succès ou un échec.
3. Si une compilation a échoué, l'édition de liens est annulée et le programme renvoie 1.
4. Sinon, un dernier enfant exécute `gcc -o executable objects...`.

Les avertissements sont affichés mais ne sont pas des erreurs, comme l'exige l'énoncé. Les exemples d'échec couvrent :
- une erreur de syntaxe (`bad_syntax.c`) ;
- un en-tête manquant (`bad_include.c`) ;
- un symbole non défini détecté à l'édition de liens (`missing_symbol.c`).

Les fichiers objets sont écrits dans le répertoire courant. Un système de build de production les placerait dans un répertoire dédié et utiliserait les horodatages pour éviter de recompiler des fichiers inchangés ; l'exercice se concentre sur la gestion des processus.

---

## Compilation, exécution et tests

```bash
make -C tp05
./tp05/bin/simultane
./tp05/bin/successif
./tp05/bin/miniature images /tmp/thumbs
./tp05/bin/telecharge https://example.com/a.txt https://example.com/b.txt /tmp/downloads
./tp05/bin/make tp05/samples/success/*.c /tmp/demo
python3 -m pytest tests/test_tp05.py -q
```

Les tests vérifient l'ordre des commandes successives, le nombre borné de conversions et de téléchargements simultanés (avec un faux `convert` et un serveur HTTP local), ainsi que les chemins de succès et d'échec du `make` miniature.
