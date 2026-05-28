# TP 3 — Traitement d'images et automates à états

## Objectifs

- manipuler un format d'image binaire (PPM) et son tableau de pixels à une dimension ;
- implémenter une convolution 3×3 (flou par moyenne) avec une politique de bord correcte ;
- concevoir et implémenter des automates finis déterministes pour un digicode, y compris un code contenant un chiffre répété et un code validé par des lettres.

---

## 1. PPM et le tableau de pixels

Une image PPM est soit ASCII (`P3`), soit binaire (`P6`). L'en-tête contient le nombre magique, la largeur, la hauteur et la valeur maximale des canaux (255 ici). Les données de pixels sont une suite de triplets RGB.

L'image est stockée dans un unique `unsigned char *` :

```
offset(x, y, channel) = 3 * (y * width + x) + channel
```

Pour une image 4×3, `(0,0) -> 0`, `(1,0) -> 3`, `(0,1) -> 12`. Un tableau à une dimension est bien plus facile à passer aux fonctions qu'un `unsigned char **`, et il correspond à la disposition mémoire du fichier.

`image_load` accepte à la fois P3 et P6, ignore les commentaires et les espaces, et vérifie chaque dimension et chaque valeur de pixel. `image_save` écrit toujours en P6.

---

## 2. Flou par moyenne

Le noyau est

```
1/9 * [ 1 1 1
        1 1 1
        1 1 1 ]
```

Pour chaque pixel et chaque canal, additionner les neuf valeurs et diviser par neuf. Par exemple, un pixel dont les voisins totalisent 1215 en rouge devient `135`.

### Politique de bord : recadrage du noyau

Au bord, certains voisins n'existent pas. Au lieu d'inventer des valeurs, l'implémentation les ignore et divise par le nombre de pixels réellement utilisés :

- coin : 4 pixels ;
- bord (hors coin) : 6 pixels ;
- intérieur : 9 pixels.

C'est la stratégie de *recadrage du noyau* exigée par l'énoncé. La sortie a les mêmes dimensions que l'entrée, et une image uniforme reste inchangée. Les autres stratégies (recadrer la sortie, étendre, refléter, enrouler) sont listées dans l'énoncé ; le recadrage du noyau est la plus simple qui maintienne défini chaque pixel de sortie.

La complexité est `O(width × height)` avec une constante d'au plus neuf additions par canal et par pixel.

---

## 3. Digicode

### Variante 1 — code 5742

L'automate possède cinq états :

```
        5          7          4          2
 (0) -------> (1) ----> (2) ----> (3) ----> ((4)) porte ouverte
  ^            |         |         |
  |  autre     | autre   | autre   | autre
  +------------+---------+---------+
      5 depuis 1, 2 ou 3 ramène à l'état 1
```

Un chiffre erroné autre que `5` remet l'automate à l'état 0. Un `5` redémarre toujours un code possible, ce qui correspond exactement au comportement de la fonction d'échec de l'automate KMP pour le motif `5742`.

### Variante 2 — code 5551

Le chiffre répété constitue la difficulté : après `55`, un nouveau `5` ne doit pas remettre l'automate à zéro, il doit conserver le plus long préfixe valide. La fonction d'échec de KMP donne :

| état | signification | sur `5` | sur `1` | autre |
|---|---|---|---|---|
| 0 | rien | 1 | 0 | 0 |
| 1 | `5` | 2 | 0 | 0 |
| 2 | `55` | 3 | 0 | 0 |
| 3 | `555` | 3 | 4 (ouvert) | 0 |

Ceci est implémenté de manière générique par `automate_init`, de sorte que le même code gère `5742`, `5551` et tout autre code composé uniquement de chiffres.

### Variante 3 — code 8267B

Ici, `A` et `B` valident une tentative. Le code n'est valide que si les quatre chiffres `8267` ont été saisis depuis la lettre de validation précédente :

- `8267B` → valide ;
- `78267B` → invalide, car le `7` initial fait partie de la même tentative ;
- `7A8267B` → la première tentative `7A` est invalide, la seconde tentative `8267B` est valide.

L'implémentation accumule au plus quatre chiffres. Un cinquième chiffre marque la tentative comme trop longue, de sorte qu'elle ne peut plus correspondre. `A` ou `B` compare les chiffres accumulés avec `8267` et la lettre avec `B`, affiche `Code valide` ou `Code invalide`, et démarre une nouvelle tentative.

### Table de transitions

`bin/table` affiche la table de transitions KMP pour `5742` et `5551` ainsi qu'une description lisible de l'automate des tentatives `8267B`. C'est la réponse écrite à la partie « dessiner l'automate » de l'énoncé.

---

## Compilation, exécution et tests

```bash
make -C tp03
./tp03/bin/flou tp03/sample.ppm /tmp/out.ppm
printf '1235742' | ./tp03/bin/digicode 5742
printf '125551'  | ./tp03/bin/digicode 5551
printf '7A8267B' | ./tp03/bin/digicode 8267B
./tp03/bin/table
python3 -m pytest tests/test_tp03.py -q
```

Les tests utilisent des images P3 et P6 générées, vérifient les valeurs du recadrage du noyau sur un coin, un pixel intérieur et une image uniforme, et exercent chaque variante de digicode, y compris les codes qui se chevauchent et les règles de validation `A`/`B`.
