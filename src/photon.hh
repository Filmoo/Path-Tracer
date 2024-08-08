#ifndef PHOTON_HH
#define PHOTON_HH

#include "vector3.hh"
#include "color.hh"

class Photon {
    public:
        Point3 position;
        Vector3 direction;
        Color power;

        Photon(Point3 pos, Vector3 dir, Color pow) : position(pos), direction(dir), power(pow) {}
};

#endif