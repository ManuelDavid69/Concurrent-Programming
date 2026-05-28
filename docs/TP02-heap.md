# TP 2 — Tas binaire et tri par tas

## Objectifs

- implémenter un tas-min avec un tableau (`entasser`, `detasser`) ;
- respecter les deux règles structurelles : ordre du tas et arbre complet ;
- afficher le tas sous plusieurs formats lisibles ;
- utiliser le tas pour trier (`tri par tas`) ;
- comparer l'implémentation par tableau avec un arbre alloué dynamiquement et un tri par tas en place.

---

## Représentation

Le tas est un arbre binaire complet stocké dans un tableau. Pour un nœud à l'indice `i` :

```
parent      = (i - 1) / 2
left child  = 2 * i + 1
right child = 2 * i + 2
```

Le tableau possède une capacité fixe (le nombre de valeurs à insérer) et une taille courante. Au début `size = 0` ; après chaque insertion elle augmente d'un ; chaque extraction la réduit d'un. Garder `size` distinct de `capacity` rend explicites les cas « plein » et « vide ».

Comme l'arbre est complet, il n'y a pas de trous : le dernier élément se trouve toujours à `size - 1`, et la prochaine insertion va exactement à cet emplacement. C'est ce qui rend la représentation par tableau à la fois compacte et favorable au cache.

---

## Insertion (`entasser`)

1. écrire la nouvelle valeur à l'indice `size` et incrémenter `size` ;
2. tant que la valeur est plus petite que son parent, les échanger ;
3. s'arrêter à la racine ou dès que le parent est plus petit ou égal.

Le chemin d'une feuille à la racine a une longueur `floor(log2 n)`, donc le pire cas est `O(log n)`. Le nombre d'échanges est le nombre de fois où la valeur remonte, au plus la hauteur de l'arbre.

## Extraction (`detasser`)

1. sauvegarder la racine (le minimum) ;
2. déplacer le dernier élément à la racine et décrémenter `size` ;
3. le faire descendre : le comparer avec ses deux enfants, l'échanger avec le plus petit enfant tant que celui-ci est plus petit que la valeur courante.

Là encore, le coût est `O(log n)`. Notons qu'après avoir déplacé le dernier élément à la racine, un seul sous-arbre peut violer la propriété de tas à chaque étape, donc un seul chemin descendant suffit.

Le vérificateur de validité (`tas_est_valide`) teste chaque paire parent/enfant. Il est utilisé par la démonstration avant l'affichage ou l'extraction, et c'est la propriété sur laquelle s'appuient les tests pour les entrées aléatoires.

---

## Affichage

Trois formats sont disponibles :

| Mode | Sortie |
|---|---|
| `line` | le tableau sous-jacent sur une ligne |
| `levels` | une ligne par niveau de profondeur |
| `tree` | un arbre ASCII indenté avec les branches `\|--` et `\--` |

Exemple :

```
38
|-- 39
|   |-- 40
|   |   |-- 55
|   |   \-- 51
|   \-- 49
\-- 42
    |-- 62
    \-- 48
```

---

## Tri par tas

Le tas utilisé pour les démonstrations est un tas-**min**, qui extrait les valeurs par ordre croissant. Pour un tri en place, `tri_par_tas` construit un tas-**max** et échange répétitivement la racine avec le dernier élément du préfixe courant. Le maximum va à sa position finale, le préfixe rétrécit, et la nouvelle racine est amenée à descendre.

```
build max-heap:      O(n)
n - 1 extractions:   O(n log n)
total:               O(n log n), in place, not stable
```

Construire le tas de bas en haut est linéaire : la plupart des nœuds sont proches des feuilles et descendent très peu. C'est mieux que `n` insertions successives, qui coûtent `O(n log n)`.

---

## Réflexion

### Tableau versus arbre alloué dynamiquement

| | Tableau | Nœuds chaînés (`malloc`) |
|---|---|---|
| Mémoire par élément | un `int` | un `int` + deux pointeurs + surcoût de l'allocateur |
| Localité | excellente | médiocre |
| Accès aux enfants | arithmétique | déréférencement de pointeur |
| Règle de l'arbre complet | automatique | doit être maintenue explicitement |
| Forme arbitraire | impossible | possible |
| Allocation pendant l'utilisation | aucune | une allocation par insertion |
| Gestion des échecs | une allocation initiale | chaque insertion peut échouer |

La version par tableau est plus simple, plus rapide et utilise moins de mémoire, mais elle exige de connaître à l'avance le nombre maximal d'éléments (ou d'avoir une politique de réallocation). La version chaînée permet un tas réellement non borné, au prix du parcours de pointeurs et d'une allocation par nœud.

### Tri par tas en place versus extraction dans un second tableau

Extraire un tas-min dans un second tableau produit également un résultat trié, mais nécessite `O(n)` de mémoire supplémentaire. La variante en place réutilise le tableau d'entrée : elle construit un tas-max et place chaque maximum à la fin du préfixe qui rétrécit. Le résultat est trié par ordre croissant et utilise `O(1)` de mémoire supplémentaire.

---

## Compilation, exécution et tests

```bash
make -C tp02
./tp02/bin/tas
./tp02/bin/tas --display tree
./tp02/bin/tas --random 20 42
./tp02/bin/tas --sort 7 3 9 1 5
python3 -m pytest tests/test_tp02.py -q
```

Les tests vérifient l'exemple de l'énoncé, les trois modes d'affichage, la génération aléatoire reproductible, l'extraction triée, les doublons, les valeurs négatives, un tas à un seul élément et un tri aléatoire de 200 éléments.
