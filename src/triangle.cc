#include "triangle.hh"

Triangle::Triangle(Texture_Material* texture, Point3 p1, Point3 p2, Point3 p3) 
: Object(texture), p1(p1), p2(p2), p3(p3) {}

Triangle::Triangle(Texture_Material* texture, Point3 p1, Point3 p2, Point3 p3, Vector3 n1, Vector3 n2, Vector3 n3)
: Object(texture), p1(p1), p2(p2), p3(p3), n1(n1), n2(n2), n3(n3) {
    isSmooth = true;
}



//Optimized way to calculate triangle intersection
bool Triangle::intersect(Point3 o,Vector3 v, float &t1) {
    Vector3 e1 = p2 - p1;
    Vector3 e2 = p3 - p1;   
    Vector3 h = v.cross(e2);
    float a = e1.dot(h);
    if (a > -0.00001 && a < 0.00001) return false;
    float f = 1/a;
    Vector3 s = o - p1;
    float u = f * s.dot(h);
    if (u < 0 || u > 1) return false;
    Vector3 q = s.cross(e1);
    float v1 = f * v.dot(q);
    if (v1 < 0 || u + v1 > 1) return false;
    float t = f * e2.dot(q);
    if (t > 0.00001) {
        t1 = t;
        return true;
    }
    return false;
}

Vector3 Triangle::normal(Point3 p) {
    Vector3 normal;
    if (isSmooth) {
        //Calculate barycentric coordinates
        Vector3 e1 = p2 - p1;
        Vector3 e2 = p3 - p1;
        Vector3 e3 = p - p1;

        float d00 = e1.dot(e1);
        float d01 = e1.dot(e2);
        float d11 = e2.dot(e2);
        float d20 = e3.dot(e1);
        float d21 = e3.dot(e2);
        float denom = d00 * d11 - d01 * d01;

        float v = (d11 * d20 - d01 * d21) / denom;
        float w = (d00 * d21 - d01 * d20) / denom;
        float u = 1.0f - v - w;

        Vector3 n = n1 * u + n2 * v + n3 * w;
        normal = n.normalize();        
    }
    else
    {
        Vector3 e1 = p2 - p1;
        Vector3 e2 = p3 - p1;
        normal = e1.cross(e2).normalize();
    }
    return normal;
}

void Triangle::getTextureMaterial(Point3 p, Color &c, float &kd, float &ks, float &shine, float &iorefrac, float &ioreflec)
{
    texture->getTextureMaterial(p, c, kd, ks, shine, iorefrac, ioreflec);
}

Point3 Triangle::getLocalCoordinates(Point3 p)
{
    Point3 center = (p1 + p2 + p3) / 3;
    return p - center;
}




