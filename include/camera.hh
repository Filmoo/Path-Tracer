#ifndef CAMERA_HH
#define CAMERA_HH

#include "vector3.hh"
#include "image.hh"
#include <cmath>

class Camera {
    public:
        Vector3* C;
        Vector3* forward;
        Vector3* up;
        float alpha;
        float beta;
        float zmin;
        float fov;
        float aspectRatio;
        float halfWidth;
        float halfHeight;
        float focalLength;
        Camera();
        Camera(Vector3* C, Vector3* P, Vector3* up, 
            float alpha, float beta, float zmin, float fov, 
            float aspectRatio, float focalLength);
};

#endif