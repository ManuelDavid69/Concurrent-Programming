# TP 5 — `fork`, `exec` et parallélisme borné

| Programme | Objectif |
|---|---|
| `bin/simultane` | `du -sh /usr/share & ls -l & ps` |
| `bin/successif` | `du -sh /usr/share ; ls -l ; ps` |
| `bin/miniature` | redimensionner chaque image d'un répertoire à 10 %, avec au plus 8 processus `convert` |
| `bin/telecharge` | télécharger une liste d'URL, avec au plus 4 processus `wget` |
| `bin/make` | compiler plusieurs fichiers `.c` en parallèle et les lier si tout réussit |

```bash
make
./bin/simultane
./bin/successif
./bin/miniature images thumbs
./bin/telecharge https://example.com/a.txt https://example.com/b.txt downloads
./bin/make samples/success/main.c samples/success/mathutils.c samples/success/message.c demo
./demo
```

`miniature` et `telecharge` maintiennent un pool d'au plus 8 (respectivement 4)
enfants actifs. Lorsque le pool est plein, le père appelle `waitpid(-1, ...)`
avant de lancer l'enfant suivant. Chaque enfant terminé est associé à sa tâche
afin que le rapport nomme le bon fichier ou la bonne URL.

`make` lance un enfant `gcc -c` par source, les attend tous, et ne lie que si
chaque compilation a réussi. Les avertissements sont affichés mais ne sont pas
des erreurs, comme l'exige l'énoncé.

Le répertoire `samples/success` compile proprement ; `samples/failure` contient
une erreur de syntaxe, un en-tête manquant et un symbole non défini pour tester
les chemins d'échec.
