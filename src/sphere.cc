#include "sphere.hh"
#include <math.h>

Sphere::Sphere(Texture_Material *texture, Point3 center, float radius)
    : Object(texture), center(center), radius(radius * radius) 
    {
        radius2 = this->radius * this->radius;
    }


//Determine if a ray intersects with the sphere
bool Sphere::intersect(Point3 o, Vector3 v, float &t)
{
    Vector3 L = center - o;
    float tca = L.dot(v);

    if (tca < 0.0)
    {
        return false;
    }
    float d2 = L.dot(L) - tca * tca;

    if (d2 > radius2)
    {
        return false;
    }
    
    double thc = sqrt(radius - d2);

    float t1,t2;
    t1 = tca - thc;
    t2 = tca + thc;
    if (t1 > t2)
    {
        float temp = t1;
        t1 = t2;
        t2 = temp;
    }
    if (t1 < 0) 
    {   
        t1 = t2;
        if (t1 < 0) 
            return false;
    }
    t = t1;
    return true;
}


//Determine the normal of the sphere at a given point
Vector3 Sphere::normal(Point3 p)
{
    return (p - center).normalize();
}

//Determine the texture and material at a given point
void Sphere::getTextureMaterial(Point3 p, Color &c, float &kd, float &ks, float &shine, float &iorefrac, float &ioreflec)
{
    texture->getTextureMaterial(p, c, kd, ks, shine, iorefrac, ioreflec);
}

Point3 Sphere::getLocalCoordinates(Point3 p)
{
    return (p - center) / (sqrt(radius));
}