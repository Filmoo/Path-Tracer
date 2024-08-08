#include "camera.hh"
//Defaut constructor
Camera::Camera()
{
    C = Point3(0, 0, 0);
    forward = Vector3(1, 0, 0);
    up = Vector3(0, 0, 1);
    alpha = 20;
    beta = 20;
    zmin = 1;
    fov = 90;
    aspectRatio = 16/9;
    halfWidth = 0;
    halfHeight = 0;
}

Camera::Camera(Point3 C, Point3 P, Vector3 up, float alpha, float beta, float zmin, float fov, float aspectRatio, float focalLength)
    : C(C), alpha(alpha), beta(beta), zmin(zmin), fov(fov), aspectRatio(aspectRatio), focalLength(focalLength)
    {
        forward = Vector3(P - C).normalize();
        float theta = fov * M_PI / 180;
        halfHeight = std::tan(theta   / 2);
        halfWidth = halfHeight * aspectRatio;
        this->up = up.normalize() * halfHeight;

    }

