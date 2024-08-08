#ifndef CAMERA_HH
#define CAMERA_HH

#include "vector3.hh"
#include "image.hh"
#include <cmath>

class Camera {
    public:
        Point3 C;
        Vector3 forward;
        Vector3 up;
        float alpha;
        float beta;
        float zmin;
        float fov;
        float aspectRatio;
        float halfWidth;
        float halfHeight;
        float focalLength;
        Camera();
        Camera(Point3 C, Point3 P, Vector3 up, 
            float alpha, float beta, float zmin, float fov, 
            float aspectRatio, float focalLength);
        Vector3 getViewDirection(Point3 p);
};

#endif