#include "plane.hh"

Plane::Plane(Texture_Material *texture, Point3 center, Vector3 normal) 
        : Object(texture), center(center), normale(normal) {}


bool Plane::intersect(Point3 o,Vector3 v, float &t1)
{
    float d = normale.dot(center);
    float t = (d - normale.dot(o)) / normale.dot(v);
    if (t > 0.00001)
    {
        t1 = t;
        return true;
    }
    return false;
}

Vector3 Plane::normal(Point3 p)
{
    return this->normale;
}

void Plane::getTextureMaterial(Point3 p, Color &c, float &kd, float &ks, float &shine, float &iorefrac, float &ioreflec)
{
    texture->getTextureMaterial(p, c, kd, ks, shine, iorefrac, ioreflec);
}

Point3 Plane::getLocalCoordinates(Point3 p)
{
    return p - center;
}

