#ifndef TRIANGLE_HH
#define TRIANGLE_HH

#include "object.hh"
#include "texture.hh"

class Triangle : public Object {
    public:
        bool isSmooth = false;
        Point3 p1;
        Point3 p2;
        Point3 p3;
        Vector3 n1;
        Vector3 n2;
        Vector3 n3;
        Triangle(Texture_Material* texture, Point3 p1, Point3 p2, Point3 p3);
        Triangle(Texture_Material* texture, Point3 p1, Point3 p2, Point3 p3, Vector3 n1, Vector3 n2, Vector3 n3);
        bool intersect(Point3 o,Vector3 v, float &t1);
        Vector3 normal(Point3 p);
        void getTextureMaterial(Point3 p, Color &c, float &kd, float &ks, float &shine, float &iorefrac, float &ioreflec);
        Point3 getLocalCoordinates(Point3 p);
        Color emission() { return Color(0,0,0); }
};




#endif // TRIANGLE_HH