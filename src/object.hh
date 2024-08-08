#ifndef OBJECT_HH
#define OBJECT_HH

#include "texture.hh"
#include "vector3.hh"

class Object{
    public:
        Texture_Material* texture;
        //Determine if a ray intersects the object
        virtual bool intersect(Point3 o, Vector3 v, float &t) = 0;

        //Determine the normal of the object at a given point
        virtual Vector3 normal(Point3 p) = 0;

        //Determine the texture and material at a given point
        virtual void getTextureMaterial(Point3 p, Color &c, float &kd, float &ks, float &shine, float &iorefrac, float &ioreflec) = 0;

        virtual Point3 getLocalCoordinates(Point3 p) = 0;

        virtual Color emission() = 0;

        virtual bool isDiffuse(Point3 p) {
            float kd, ks, shine, iorefrac, ioreflec;
            Color c;
            getTextureMaterial(p, c, kd, ks, shine, iorefrac, ioreflec);
            return kd > 0 && ks == 0;
        }

        Object(Texture_Material* texture) : texture(texture) {} // Default constructor

};

#endif