#include "camera.hh"
#define M_PI 3.14159265358979323846
//Defaut constructor
Camera::Camera()
{
    C = new Vector3(0, 0, 0);
    forward = new Vector3(1, 0, 0);
    up = new Vector3(0, 0, 1);
    alpha = 20;
    beta = 20;
    zmin = 1;
    fov = 90;
    aspectRatio = 16/9;
    halfWidth = 0;
    halfHeight = 0;
}

Camera::Camera(Vector3* C, Vector3* P, Vector3* up, float alpha, float beta, float zmin, float fov, float aspectRatio, float focalLength)
    : C(C), alpha(alpha), beta(beta), zmin(zmin), fov(fov), aspectRatio(aspectRatio), focalLength(focalLength)
    {
        forward = P;
        float theta = fov * M_PI / 180;
        halfHeight = std::tan(theta   / 2);
        halfWidth = halfHeight * aspectRatio;
        this->up = new Vector3(up->normalize());
    }

