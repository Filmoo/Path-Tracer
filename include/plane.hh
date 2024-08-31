#ifndef PLANE_HH
#define PLANE_HH

#include "object.hh"
#include "texture.hh"

class Plane : public Object {
    public:
        Point3 center;
        Vector3 normale;
        Plane(Texture_Material* texture, Point3 center, Vector3 normal);
        bool intersect(Point3 o,Vector3 v, float &t1);
        Vector3 normal(Point3 p);
        void getTextureMaterial(Point3 p, Color &c, float &kd, float &ks, float &shine, float &iorefrac, float &ioreflec);
        Point3 getLocalCoordinates(Point3 p);
        Color emission() { return Color(0,0,0); }
};

#endif 