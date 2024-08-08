#include "box.hh"

Box::Box(Texture_Material* texture, Point3 minCorner, Point3 maxCorner) 
    :Object(texture), minCorner(minCorner), maxCorner(maxCorner) {}

bool Box::intersect(Point3 o, Vector3 v, float &t1)
{
    float tMin = (minCorner.x - o.x) / v.x;
    float tMax = (maxCorner.x - o.x) / v.x;

    if (tMin > tMax) std::swap(tMin, tMax);

    float tyMin = (minCorner.y - o.y) / v.y;
    float tyMax = (maxCorner.y - o.y) / v.y;

    if (tyMin > tyMax) std::swap(tyMin, tyMax);

    if ((tMin > tyMax) || (tyMin > tMax)) return false;

    if (tyMin > tMin) tMin = tyMin;

    if (tyMax < tMax) tMax = tyMax;

    float tzMin = (minCorner.z - o.z) / v.z;
    float tzMax = (maxCorner.z - o.z) / v.z;

    if (tzMin > tzMax) std::swap(tzMin, tzMax);

    if ((tMin > tzMax) || (tzMin > tMax)) return false;

    if (tzMin > tMin) tMin = tzMin;

    if (tzMax < tMax) tMax = tzMax;

    t1 = tMin;

    return true;
}

Vector3 Box::normal(Point3 p)
{
    if (p.x == minCorner.x)
    {
        return Vector3(-1, 0, 0);
    }
    else if (p.x == maxCorner.x)
    {
        return Vector3(1, 0, 0);
    }
    else if (p.y == minCorner.y)
    {
        return Vector3(0, -1, 0);
    }
    else if (p.y == maxCorner.y)
    {
        return Vector3(0, 1, 0);
    }
    else if (p.z == minCorner.z)
    {
        return Vector3(0, 0, -1);
    }
    else if (p.z == maxCorner.z)
    {
        return Vector3(0, 0, 1);
    }
    else
    {
        std::cout << "p: " << p << " minCorner: " << minCorner << " maxCorner: " << maxCorner << std::endl; 
        std::cout << "Error: Box normal not found" << std::endl;
        return Vector3(0, 0, 0);
    }
}

void Box::getTextureMaterial(Point3 p, Color &c, float &kd, float &ks, float &shine, float &iorefrac, float &ioreflec)
{
    texture->getTextureMaterial(p, c, kd, ks, shine, iorefrac, ioreflec);
}

Point3 Box::getLocalCoordinates(Point3 p)
{
    return p;
}