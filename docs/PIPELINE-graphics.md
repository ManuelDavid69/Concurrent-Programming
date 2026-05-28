# Pipeline graphique — threads et barrières

## Objectifs

- construire un pipeline producteur/consommateur à plusieurs étages ;
- relier les étages par des tampons bornés ;
- synchroniser tous les étages avec une barrière lorsque la scène change ;
- rendre une image PPM complète à partir d'une scène 3D procédurale.

---

## 1. Architecture

```
                 triangles                     fragments
creator thread -----------> raster threads -----------> shader threads --> image
                buffer                        buffer
```

| Étage | Threads | Entrée | Sortie |
|---|---|---|---|
| créateur de triangles | 1 | tore procédural | tampon `Triangle` |
| rastériseur | R | triangles | tampon `Fragment` |
| shader | S | fragments | image + z-buffer |

`R` et `S` sont des arguments de ligne de commande. Les files sont bornées afin de maintenir une utilisation mémoire constante et d'appliquer une contre-pression au producteur.

`gpu.c` fournit les primitives graphiques et n'est pas modifié :

```c
void *triangle_creator_init(void);
int triangle_creator_get_next_triangle(void *creator, struct Triangle *triangle);
void triangle_creator_destroy(void *creator);
int triangle_is_sync(struct Triangle *triangle);

void *raster_init(const struct Triangle *triangle);
int raster_get_next_fragment(void *raster_state, struct Fragment *fragment);
void raster_destroy(void *raster_state);
struct Fragment fragment_sync(void);
int fragment_is_sync(struct Fragment *fragment);

void gpu_shader(struct Fragment fragment);
```

`triangle_creator_get_next_triangle` renvoie 0 à la fin de la scène. `triangle_is_sync` identifie un marqueur de changement de couleur. `raster_init` suivi d'appels répétés à `raster_get_next_fragment` énumère les pixels couverts par un triangle (un fragment par pixel). `gpu_shader` effectue le test de profondeur (z-test) et écrit le pixel.

---

## 2. Scène procédurale

La scène est un tore généré avec 28 × 14 quadrilatères, soit 784 triangles. Tous les 112 triangles, la couleur change et un triangle de synchronisation est inséré. Le tore est tourné autour des axes y et x et projeté orthogonalement sur une image 640×480.

Les six couleurs de la palette et le fond sombre expliquent pourquoi l'image finale ne contient que sept valeurs RVB distinctes : le shader utilise des couleurs plates, sans éclairage interpolé. Le but du TP est le pipeline, pas un moteur de rendu physiquement réaliste.

---

## 3. Synchronisation du pipeline

### Files

Les deux tampons sont des tableaux circulaires protégés par un mutex et deux variables de condition par file (`not_empty`, `not_full`). Un producteur attend lorsque la file est pleine ; un consommateur attend lorsqu'elle est vide.

### Barrières de changement de couleur

Un pipeline producteur/consommateur ordinaire n'a aucune raison d'attendre : des fragments de l'ancienne couleur peuvent encore être dans les tampons lorsque le créateur commence à produire la nouvelle couleur, ce qui provoque un déchirement (tearing) visible. Le triangle de synchronisation déclenche un **vidage global** :

1. le créateur incrémente `epoch`, diffuse toutes les conditions et attend sur une barrière de `1 + R + S` threads ;
2. chaque thread de rastérisation remarque la nouvelle époque (epoch), cesse de prendre de nouveaux triangles, termine les triangles déjà en cours (`raster_busy`), vide la file de triangles, incrémente `raster_done`, puis attend à la barrière ;
3. chaque thread de shader remarque la nouvelle époque, continue de consommer des fragments jusqu'à ce que `raster_done == R` et que la file de fragments soit vide, incrémente `shader_done`, puis attend à la barrière ;
4. lorsque tous les threads sont arrivés, la barrière les libère et le créateur remet les compteurs à zéro et continue avec la nouvelle couleur.

Le compteur `raster_busy` est essentiel : un triangle peut avoir été retiré de la file mais pas encore entièrement rastérisé lorsque l'époque change. Le compter empêche les shaders de déclarer le pipeline vide trop tôt.

Utiliser une barrière (plutôt qu'un compteur mutex/condition) rend aussi le rendez-vous réutilisable : chaque thread appelle `pthread_barrier_wait` exactement une fois par changement de couleur, et la barrière se réinitialise automatiquement pour le tour suivant.

### Fin de la scène

Après le dernier triangle, le créateur effectue une dernière synchronisation. Cela garantit que chaque fragment a été ombré avant que `triangle_creator_destroy` n'enregistre l'image. Ensuite, le drapeau `done` est positionné, les conditions sont diffusées, et tous les threads sont joints.

---

## 4. Rastérisation et ombrage

Pour chaque triangle, `raster_init` calcule la boîte englobante et l'aire signée. Pour chaque centre de pixel à l'intérieur du triangle, des poids barycentriques donnent la couleur et la profondeur interpolée. `gpu_shader` compare la profondeur avec le z-buffer sous un mutex et écrit le pixel s'il est plus proche.

Le z-buffer est la seule structure partagée écrite par les shaders ; le mutex rend la comparaison-écriture atomique. L'image elle-même est écrite en même temps que la profondeur, donc aucun autre verrou n'est nécessaire.

---

## 5. Résultats

- `pipeline 1 1` — configuration minimale, plus facile à déboguer ;
- `pipeline 2 2` — la configuration par défaut ;
- `pipeline 4 3` — davantage de parallélisme.

Toutes les configurations produisent une image PPM P6 640×480 avec les mêmes sept couleurs et la même silhouette de tore. Les valeurs de pixels exactes sont déterministes pour une configuration donnée ; des nombres de threads différents peuvent différer sur les arêtes de triangles partagées, car l'ordre des fragments de profondeur égale n'est pas spécifié.

L'image générée est présentée ci-dessous. La barrière supprime les bandes de couleur horizontales qui apparaissent lorsque les étages ne sont pas synchronisés.

![Tore rendu par le pipeline](images/pipeline.png)

---

## Compilation, exécution et tests

```bash
make -C pipeline
./pipeline/bin/pipeline 2 2 pipeline.ppm
./pipeline/bin/pipeline 1 1 small.ppm
python3 -m pytest tests/test_pipeline.py -q
```

Les tests exécutent le pipeline avec 1/1, 2/2 et 4/3 threads, vérifient les dimensions PPM, le nombre de couleurs et le nombre de pixels non-fond, et s'assurent qu'un nombre de threads nul est rejeté.
