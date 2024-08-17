#include "light.hh"

Light::Light(Color c, Point3 p, float intensity) 
    :color(c), origin(p), intensity(intensity){}

Point_Light::Point_Light(Color c, Point3 p, float intensity) 
    : Light(c, p, intensity){}
    
Point3 Point_Light::getOrigin()
{
    return origin;
}
Color Point_Light::getLight(Point3 p)
{
    return color;
}


Sphere_Light::Sphere_Light(Color c, Point3 p, float r, float intensity) 
    : Light(c, p, intensity), radius(r * r), Object(new Emitter_Texture(c)){}

Color Sphere_Light::getLight(Point3 p)
{
    return color;
}


Point3 Sphere_Light::getOrigin() {
    double theta = random_double(seed) * 2 * M_PI;
    double phi = acos(2 * random_double(seed) - 1);
    double x = radius * sin(phi) * cos(theta);
    double y = radius * sin(phi) * sin(theta);
    double z = radius * cos(phi);
    return Point3(x, y, z) + origin;
}

bool Sphere_Light::intersect(Point3 o, Vector3 v, float &t)
{
    Vector3 L = origin - o;
    float tca = L.dot(v);

    if (tca < 0.0)
    {
        return false;
    }
    
    float d2 = L.dot(L) - tca * tca;
    double radius2 = radius * radius;

    if (d2 > radius2)
    {
        return false;
    }
    
    double thc = sqrt(radius - d2);
    float t1 = tca - thc;
    float t2 = tca + thc;

    if (t1 > t2)
    {
        std::swap(t1, t2);
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

Vector3 Sphere_Light::normal(Point3 p)
{
    return (p - origin).normalize();
}

void Sphere_Light::getTextureMaterial(Point3 p, Color &c, float &kd, float &ks, float &shine, float &iorefrac, float &ioreflec)
{
    texture->getTextureMaterial(p, c, kd, ks, shine, iorefrac, ioreflec);   
}

Point3 Sphere_Light::getLocalCoordinates(Point3 p)
{
    return p - origin;
}
