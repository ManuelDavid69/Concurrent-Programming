# Pipeline graphique — threads et barrières

Un pipeline de rendu logiciel à trois étapes :

```
triangle creator -> triangle buffer -> rasterisers -> fragment buffer -> shaders -> image
```

```bash
make
./bin/pipeline 2 2 pipeline.ppm
./bin/pipeline 1 1 small.ppm
```

- un thread créateur de triangles (le thread principal) génère un tore procédural ;
- plusieurs threads rastériseurs transforment les triangles en fragments ;
- plusieurs threads shaders appliquent le z-buffer et écrivent les pixels ;
- des files bornées relient les étapes.

À chaque fois que la scène change de couleur, le créateur insère un point de
synchronisation. Toutes les étapes vident leurs files et se retrouvent à une
`pthread_barrier_t`, ce qui garantit qu'aucun triangle ni fragment de la couleur
précédente n'est ombré après le changement. C'est ce qui supprime le déchirement
(tearing) visible dans la version non synchronisée.

`gpu.c` contient les primitives graphiques (génération de scène, rastérisation et
ombrage) et ne doit pas être modifié ; seul `pipeline.c` contient la logique de
synchronisation.

La sortie est un fichier PPM P6 de 640×480. Le nombre de threads rastériseurs et
shaders est configurable ; les deux doivent être au moins égaux à un.
