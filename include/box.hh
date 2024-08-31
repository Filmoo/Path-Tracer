#ifndef BOX_HH
#define BOX_HH

#include "object.hh"
#include "texture.hh"

class Box : public Object {
    public:
        Point3 minCorner;
        Point3 maxCorner;
        Box(Texture_Material* texture, Point3 minCorner, Point3 maxCorner);
        bool intersect(Point3 o,Vector3 v, float &t1);
        Vector3 normal(Point3 p);
        void getTextureMaterial(Point3 p, Color &c, float &kd, float &ks, float &shine, float &iorefrac, float &ioreflec);
        Point3 getLocalCoordinates(Point3 p);
};

#endif