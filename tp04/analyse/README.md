# Analyse du code — réponses

Les trois programmes sont compilés en `bin/analyse-cas1`, `bin/analyse-cas2`,
`bin/analyse-cas2-ordonne` et `bin/analyse-cas3`. Le raisonnement complet se
trouve dans [`../../docs/TP04-arguments-fork.md`](../../docs/TP04-arguments-fork.md) ;
cette page est la fiche de réponses courte.

## Cas 1

- Le bloc `if` s'exécute dans le **père** (`fork()` a renvoyé un pid positif).
- L'ordre n'est pas déterministe. Sortie possible : `8 6 4 2 0` (toute
  permutation de l'ensemble `{0, 2, 4, 6, 8}` est possible en principe).
- Pour le rendre déterministe, il faut attendre l'enfant avant d'afficher ; le
  fils le plus profond affiche alors `0` en premier : `0 8 6 4 2`.
- `fork()` peut échouer avec `EAGAIN` (limite de processus) ou `ENOMEM`.

## Cas 2

- Arbre de processus : une chaîne `A -> B -> C -> D -> E`.
- Non déterministe à cause du `sleep` aléatoire.
- Pour afficher `E D C B A`, chaque père attend son fils avant d'afficher sa
  propre lettre (voir `cas2_ordonne.c`).

## Cas 3

- Le processus initial crée quatre enfants et les attend tous.
- Chaque enfant additionne un quart de `tab` ; les quarts totalisent `100`,
  `125`, `100`, `125`.
- L'ordre des quatre sommes n'est pas déterministe.
- `Calculs terminés` est affiché après les quatre `wait`, il est donc en
  dernier.

## Conclusion générale

`fork()` duplique le processus, y compris l'état courant de sa boucle. Sans
condition, une boucle crée un nombre exponentiel de processus. `wait` et
`waitpid` sont nécessaires pour récupérer les enfants et pour ordonner un père
après ses enfants ; l'ordonnancement rend de toute façon la sortie concurrente
non déterministe tant qu'aucune synchronisation explicite n'est ajoutée.
