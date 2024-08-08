# Path Tracer


Pour modifier le nombre d'échantillons pas pixel, il faut le faire dans le fichier scene.hh
Pour modifier la taille de l'image il faut modifier la width dans scene.cc (~ ligne 32)


# Building

cd path-tracer
cmake ..
make
./main

# Results
<figure>
  <figcaption>2k samples | 10 depth</figcaption>
  <img
  src="https://github.com/Filmoo/Path-Tracer/blob/main/finalv2.png"
  alt="A beautiful path tracer rendered image (:">
</figure>
It took 30min for the image to be rendered on a CPU i7 10th gen.

<figure>
  <figcaption>Close up on indirect lighting</figcaption>
  <img
  src="https://github.com/Filmoo/Path-Tracer/blob/main/indrect1.png"
  alt="A beautiful path tracer rendered image (:">
</figure>

