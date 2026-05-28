# TP 6 — Tuyaux anonymes

| Programme | Objectif |
|---|---|
| `bin/guess` | jeu de devinette de nombres : le père pose les questions, l'enfant détient le secret |
| `bin/redirection` | `wc < /etc/passwd`, `cut | tr`, et le pipeline complet `cut | sed | sort > users` |
| `bin/somme` | générateur + filtre pair/impair + deux additionneurs, reliés par cinq tuyaux |

```bash
make
printf '10\n20\n30\n40\n42\n' | GUESS_SECRET=42 ./bin/guess
./bin/redirection simple
./bin/redirection pipe
./bin/redirection pipeline
./bin/somme --verbose 20 42
```

`guess` utilise deux tuyaux : un pour les propositions et un pour la réponse
(`+`, `-` ou `=`). `GUESS_SECRET` rend les tests déterministes ; sans lui,
l'enfant tire un nombre aléatoire entre 1 et 1000.

`redirection` illustre `dup2` sur l'entrée/sortie standard. Le pipeline est
construit avec trois enfants et deux tuyaux ; seul le premier enfant connaît
`/etc/passwd`, et seul le dernier connaît `users`.

`somme` ferme chaque extrémité de tuyau inutilisée dans chaque processus, ce qui
est essentiel pour éviter les lectures bloquantes. Le générateur envoie `-1`
comme marqueur de fin ; le filtre le transmet aux deux additionneurs, qui
additionnent leurs valeurs et renvoient une somme au générateur.
