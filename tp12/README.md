# TP 12 — Threads POSIX

| Programme | Objectif |
|---|---|
| `bin/rendezvous_sem` | rendez-vous de N threads avec un mutex et une variable de condition |
| `bin/rendezvous_barrier` | le même rendez-vous avec `pthread_barrier_t` |
| `bin/prodcons` | producteur/consommateur borné avec plusieurs threads |
| `bin/echange` | échange de valeurs à trois threads |
| `bin/fractale` | moteur de rendu Mandelbrot multithreadé |

```bash
make
./bin/rendezvous_sem 5
./bin/rendezvous_barrier 5
./bin/prodcons 2 2 3 20
./bin/echange
./bin/fractale 4 800 600 fractale.ppm
```

`rendezvous_sem` utilise un compteur de génération pour que la variable de
condition ne puisse pas réveiller un thread du tour suivant.
`rendezvous_barrier` est le même algorithme délégué à la barrière pthread.

`prodcons` utilise un tampon circulaire, un mutex et deux variables de condition.
Le dernier producteur diffuse `not_empty` après avoir positionné
`producers_done`, ce qui permet à chaque consommateur de sortir une fois le
tampon vide.

`echange` utilise trois sémaphores et le même protocole P0/P1/P2 que le TP 10,
mais les tables et les trois cellules sont dans le même espace d'adressage. Les
threads partagent directement les variables ; seul l'ordonnancement nécessite
des sémaphores.

`fractale` répartit les lignes entre N threads. Chaque thread appelle la fonction
`calcul` sur ses propres lignes, donc aucun verrou n'est nécessaire : deux
threads n'écrivent jamais le même pixel. Lancez-le avec 1 et 4 threads et
comparez la sortie de `time`.
