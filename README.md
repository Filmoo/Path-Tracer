# Path Tracer | DirectX 11 + Win32 API

Pour modifier le nombre d'échantillons pas pixel, il faut le faire dans le fichier scene.hh
Pour modifier la taille de l'image il faut modifier la width dans scene.cc (~ ligne 32)


# Building
- `cd path-tracer`
- `cmake ..`
- `make`
- `./main`

# Features

## UI
- [x] Real time rendering
- [x] Camera movement
- [ ] Add/Modify Objects
- [ ] Real-time Object Transformation
- [ ] Material Editor
- [ ] Camera Settings Panel
- [ ] Path Tracer Settings Panel
## Path Tracer
- [x] Multiple Light Sources
- [ ] Mesh and Objects
- [ ] Adaptive Sampling
- [ ] Depth of Field
- [ ] Motion Blur
- [ ] Volumetric Scattering
- [ ] Subsurface Scattering
- [ ] Photon Mapping
- [ ] Denoising
## Opti
- [x] Multi-threading
- [ ] GPU Acceleration
- [ ] BVH Acceleration Structures

# Results
<figure>
  <img
  src="https://github.com/Filmoo/Path-Tracer/blob/main/finalv2.png"
  alt="A beautiful path tracer rendered image (:">
  <figcaption>2k samples | 10 depth</figcaption>

</figure>
It took 30min for the image to be rendered on a CPU i7 10th gen.

<figure>
  <img
  src="https://github.com/Filmoo/Path-Tracer/blob/main/indrect1.png"
  alt="A beautiful path tracer rendered image (:">
  <figcaption>Close up on indirect lighting</figcaption>
</figure>

