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
  <img
  src="https://github.com/Filmoo/Path-Tracer/blob/main/finalv2.png"
  alt="A beautiful path tracer rendered image (:">
  <figcaption>2k samples | 10 depth | 30min rendering on CPU</figcaption>
</figure>


Close up on indirect lighting
![Close up on indirect lighting](https://github.com/Filmoo/Path-Tracer/blob/main/indrect1.png)

