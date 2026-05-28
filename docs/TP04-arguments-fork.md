# TP 4 — Arguments, `fork` et `wait`

## Objectifs

- lire et valider les arguments de la ligne de commande ;
- comprendre que `fork()` duplique le processus courant, y compris son pointeur d'instruction, ses variables et ses descripteurs ouverts ;
- dessiner des arbres de processus ;
- utiliser `wait`/`waitpid` pour récupérer les enfants et collecter les codes de sortie ;
- rendre un affichage concurrent déterministe sans renoncer au parallélisme.

---

## 1. `miroir` et `moyenne`

`miroir` inverse chaque argument en place à l'aide de deux indices. Il modifie directement `argv[i]`, ce qui est autorisé car les chaînes sont modifiables dans un `main` normal.

`moyenne` utilise l'analyseur strict de `common` et rejette tout ce qui n'est pas un entier dans `[0, 20]`. En particulier, `3.5`, `toto` et `22` produisent `Note(s) non valide(s)` ; aucun argument produit le message de l'énoncé. La moyenne est calculée en `double` et affichée avec `%.2f`.

---

## 2. `fork` dans une boucle (`boucle`)

Le programme appelle `fork()` trois fois sans `if` ni `break`. Chaque processus poursuit donc la boucle, et le nombre de processus double à chaque itération.

```
iteration 0:            P0
                         |
iteration 1:            P0 -- C1
                         |      |
iteration 2:      P0 -- C2     C1 -- C3
```

L'**arbre de processus** est un arbre binaire complet de profondeur 3 : 8 processus au total. Chaque processus affiche une ligne à chaque itération qu'il exécute :

| Créé à | Affiche à | Nombre de lignes |
|---|---|---|
| avant la boucle (racine) | 0, 1, 2 | 3 |
| itération 0 (1 processus) | 0, 1, 2 | 3 |
| itération 1 (2 processus) | 1, 2 | 4 |
| itération 2 (4 processus) | 2 | 4 |
| **total** | | **14** |

L'entrelacement exact des 14 lignes dépend de l'ordonnanceur. Les lignes de la racine ont le même pid et des compteurs de boucle différents ; la première ligne d'un enfant a le même compteur que l'itération durant laquelle il a été créé par fork, et sa valeur de retour de `fork` vaut `0`.

`wait(NULL)` à la fin ne récupère que les enfants directs du processus ; comme chaque processus exécute cette boucle, tout l'arbre est récupéré.

---

## 3. `wait`

Le parent crée d'abord les N enfants, puis appelle `waitpid(-1, ...)` dans une boucle. Chaque enfant :

- affiche son pid et celui de son parent ;
- dort `2 * i` secondes (mis à l'échelle par `PC_SLEEP_SCALE` dans les tests) ;
- affiche qu'il reprend ;
- se termine avec le code `i`.

Le parent affiche le pid et le statut de sortie de chaque enfant dès que l'un d'eux se termine.

**Le programme est-il déterministe ?** L'ensemble des messages et des codes de sortie est déterministe, mais l'ordre des retours de `wait` du parent n'est pas garanti : il dépend de l'ordonnancement, même si les durées de sommeil croissantes rendent l'ordre `0, 1, 2, ...` très probable. Pour le rendre totalement déterministe, le parent pourrait attendre un pid précis (`waitpid(children[i], ...)`) au lieu de `-1`.

---

## 4. Quatre enfants affichant `1..200`

### Variante 1 — concurrente

Chaque enfant affiche son propre bloc de 50 nombres. Les blocs sont corrects, mais leurs lignes peuvent s'entrelacer arbitrairement :

```
1
51
2
52
...
```

### Variante 2 — ordre garanti

Les quatre enfants sont toujours créés immédiatement, mais chacun se bloque sur un tube privé. Le parent écrit un jeton à l'enfant 0, attend sa fin, écrit un jeton à l'enfant 1, et ainsi de suite. La sortie est exactement `1, 2, ..., 200`, tandis que la création des processus reste concurrente.

Une alternative consisterait à laisser chaque enfant écrire dans un tube et à faire concaténer les tubes dans l'ordre par le parent. La solution par jetons utilise moins de mémoire et démontre l'usage d'un message de synchronisation d'un octet.

---

## 5. Analyse de code

### Cas 1

```c
for (i = 1; i < 5; i++) {
    fils_pid = fork();
    if (fils_pid > 0) {
        n = i * 2;
        break;
    }
}
printf("%d\n", n);
```

- Dans le bloc `if`, nous sommes dans le **parent** (`fork()` a renvoyé un pid positif). Le parent sort de la boucle ; l'enfant poursuit avec l'itération suivante.
- Le programme n'est **pas déterministe** dans l'ordre de ses sorties. L'ensemble des valeurs affichées est `{0, 2, 4, 6, 8}` : l'enfant le plus profond (créé à `i = 4`) quitte la boucle avec `n = 0` ; ses ancêtres affichent `8`, `6`, `4`, `2`. Une sortie possible est `0 8 6 4 2`, une autre est `8 0 6 4 2`.
- Pour le rendre déterministe, un parent peut attendre son enfant avant d'afficher :

  ```c
  if (fils_pid > 0) {
      n = i * 2;
      wait(NULL);
      break;
  }
  ```

  Avec cette modification, l'enfant le plus profond affiche en premier et la sortie est `0, 8, 6, 4, 2`.
- `fork()` peut échouer : `EAGAIN` lorsque la limite de processus est atteinte, `ENOMEM` lorsque le noyau ne peut pas allouer les structures du processus. Il renvoie `-1` et positionne `errno` ; le programme devrait tester ce cas.

### Cas 2

```c
for (i = 0; i < 4; i++)
    if (fork())
        break;
/* dormir puis afficher 'A' + i */
```

- Arbre de processus : une **chaîne** `A -> B -> C -> D -> E`. À chaque itération, le parent sort de la boucle (et affichera sa lettre), tandis que l'enfant poursuit la boucle. Le dernier enfant quitte la boucle avec `i = 4` et affiche `E`.
- Le programme n'est **pas déterministe** : chaque processus dort pendant un nombre aléatoire de secondes, donc l'ordre des cinq lignes varie.
- Pour obtenir `E D C B A` sans modifier les premières lignes, chaque parent doit attendre son enfant avant d'afficher :

  ```c
  pid_t child = fork();
  if (child > 0) {
      waitpid(child, NULL, 0);
      printf("Je suis %c\n", 'A' + i);
      break;
  }
  ```

  La feuille `E` affiche en premier, puis son parent `D`, et ainsi de suite jusqu'à `A`. La variante complète se trouve dans `analyse/cas2_ordonne.c`.

### Cas 3

- Arbre de processus : le processus initial crée quatre enfants puis les attend. Chaque enfant conserve la valeur de `i` au moment de son `fork`, sort de la boucle et additionne un quart de `tab`.
- Le `Calculs terminés` du parent est affiché **après** les quatre `wait`, c'est donc la dernière ligne.
- L'ordre des quatre lignes `i somme` n'est **pas déterministe**.
- Les quarts valent `100`, `125`, `100`, `125` pour `i = 0, 1, 2, 3` (`tab[j] = j % 10`).
- Conclusions : `fork` duplique tout le processus ; une condition est nécessaire pour donner un rôle à chaque processus ; `wait` est nécessaire pour éviter que le parent ne termine avant ses enfants ; et l'ordonnancement rend la sortie concurrente non déterministe à moins d'ajouter une synchronisation explicite.

---

## Compilation, exécution et tests

```bash
make -C tp04
./tp04/bin/miroir trace soda saper
./tp04/bin/moyenne 12 18 3
./tp04/bin/boucle
./tp04/bin/wait 4
./tp04/bin/enfants
./tp04/bin/enfants --ordered
./tp04/bin/analyse-cas1
./tp04/bin/analyse-cas2
./tp04/bin/analyse-cas2-ordonne
./tp04/bin/analyse-cas3
python3 -m pytest tests/test_tp04.py -q
```

Les tests vérifient les messages exacts, l'arbre de 8 processus de `boucle`, les 200 valeurs des deux variantes d'`enfants`, les codes de sortie collectés par `wait`, et les réponses des trois programmes d'analyse.
