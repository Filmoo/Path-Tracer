#ifndef LIGHT_HH
#define LIGHT_HH

#include "vector3.hh"
#include "color.hh"
#include "object.hh"
#include "utils.hh"
#include <cmath>

class Light {
    public:
        Vector3 direction;
        Color color;
        Point3 origin;
        float intensity;
        Light(Color color, Point3 origin, float intensity = 1.0);
        virtual Color getLight(Point3 p) = 0;
        virtual Point3 getOrigin() = 0;
};

class Point_Light : public Light {
    public:
        Point_Light(Color color, Point3 origin, float intensity = 1.0);
        Color getLight(Point3 p);
        Point3 getOrigin();
        Color emission() { return color * intensity; }
};

class Sphere_Light : public Light, public Object {
    public:
        float radius;
        Sphere_Light(Color color, Point3 origin, float radius, float intensity = 1.0);
        Color getLight(Point3 p);
        Point3 getOrigin();
        bool intersect(Point3 o, Vector3 v, float &t);
        Vector3 normal(Point3 p);
        void getTextureMaterial(Point3 p, Color &c, float &kd, float &ks, float &shine, float &iorefrac, float &ioreflec);
        Point3 getLocalCoordinates(Point3 p);
        Color emission() { 
            return color * intensity; }
};


#endif