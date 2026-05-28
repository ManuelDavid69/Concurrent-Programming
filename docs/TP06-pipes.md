# TP 6 — Tubes anonymes

## Objectifs

- créer et utiliser des tubes anonymes (`pipe`, `read`, `write`) ;
- connecter des processus et rediriger leurs descripteurs standard (`dup2`) ;
- fermer toute extrémité de tube inutilisée pour éviter les lectures bloquantes ;
- construire un filtre multi-processus avec une valeur sentinelle.

---

## 1. Le jeu de devinette (`guess`)

Le parent et l'enfant communiquent par **deux** tubes :

```
parent ---- int guess ----> child
parent <--- char reply ---- child
```

L'enfant tire un secret entre 1 et 1000. Pour chaque proposition, il écrit :

- `+` si la proposition est plus petite que le secret ;
- `-` si elle est plus grande ;
- `=` si elle est égale, puis il se termine.

Le parent affiche l'indice, lit la ligne suivante et boucle jusqu'à `=` ou EOF. `GUESS_SECRET` permet des tests reproductibles ; sinon le secret est aléatoire.

Les deux directions utilisent `write_all` et `read_all`, qui réessaient sur `EINTR` et gèrent les transferts partiels. Un seul `read` ne garantit pas de renvoyer un `int` entier, même sur un tube.

---

## 2. Redirections et pipelines (`redirection`)

### `simple` — `wc < /etc/passwd`

L'enfant ouvre `/etc/passwd`, duplique le descripteur sur l'entrée standard avec `dup2`, ferme le descripteur d'origine et exécute `wc`. `wc` ne sait pas d'où vient son entrée.

### `pipe` — `cut -f 1 -d : /etc/passwd | tr a-z A-Z`

Un tube est créé avant les deux forks. L'enfant `cut` redirige l'extrémité d'écriture vers la sortie standard ; l'enfant `tr` redirige l'extrémité de lecture vers l'entrée standard. Le parent ferme les deux extrémités, puis attend les deux enfants.

### `pipeline` — la commande complète

```
cut -f 1,3 -d : < /etc/passwd
  | sed 's+^\(.*\):\(.*\)+\2:\1+'
  | sort -n > users
```

Trois enfants et deux tubes :

| Processus | stdin | stdout |
|---|---|---|
| `cut` | `/etc/passwd` | tube 1 |
| `sed` | tube 1 | tube 2 |
| `sort` | tube 2 | `users` |

Seul `cut` ouvre `/etc/passwd` ; seul `sort` ouvre `users`. C'est exactement le comportement du shell : chaque commande ne voit que ses propres descripteurs.

**Pourquoi fermer les extrémités inutilisées ?** Si le parent (ou un autre processus) garde ouverte l'extrémité d'écriture d'un tube, le lecteur ne voit jamais l'EOF et se bloque indéfiniment dès que le producteur s'arrête. Fermer chaque extrémité inutilisée n'est pas une optimisation, c'est une exigence de correction.

---

## 3. Filtre pair/impair (`somme`)

Le programme est composé de quatre processus et cinq tubes :

```
generator --numbers--> filter --even--> pair  --sum--> generator
                              \--odd---> impair --sum--> generator
```

- le **générateur** (le processus initial) tire N nombres, les écrit au filtre, puis écrit `-1` ;
- le **filtre** lit chaque nombre et le transmet au tube pair ou impair ; lorsqu'il lit `-1`, il transmet `-1` aux deux additionneurs et se termine ;
- **pair** et **impair** additionnent leurs valeurs et renvoient un entier au générateur ;
- le générateur lit les deux sommes et affiche les totaux.

La sentinelle `-1` est valide ici car les valeurs générées sont non négatives. Chaque processus ferme toutes les extrémités de tube qu'il n'utilise pas, de sorte que le filtre ne voit l'EOF que lorsque le générateur ferme son extrémité d'écriture, et les additionneurs se terminent dès qu'ils reçoivent la sentinelle.

La somme des deux sommes partielles est aussi la somme de toutes les valeurs générées ; les tests la recalculent à partir de la sortie de `--verbose`.

---

## Compilation, exécution et tests

```bash
make -C tp06
printf '10\n20\n30\n40\n42\n' | GUESS_SECRET=42 ./tp06/bin/guess
./tp06/bin/redirection simple
./tp06/bin/redirection pipe
./tp06/bin/redirection pipeline
./tp06/bin/somme --verbose 100 42
python3 -m pytest tests/test_tp06.py -q
```

Les tests comparent les redirections avec les vraies commandes du shell, exercent le jeu de devinette avec un secret imposé, et vérifient les sommes pair/impair pour plusieurs valeurs de N, y compris le cas vide.
