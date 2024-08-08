#ifndef SCENE_HH
#define SCENE_HH

#include "vector3.hh"
#include "object.hh"
#include "light.hh" 
#include "camera.hh"
#include "utils.hh"
#include "tiny_obj_loader.h"
#include "triangle.hh"

#include <vector>
#include <limits>

class Scene {
    public:
        std::vector<Object*> objects;

        std::vector<Light*> lights;

        Camera camera;

        //Maximum depth of the ray casting
        int depth = 4;

        //Number of samples for indirect lighting
        int indirectSamples = 16;

        //Skybox
        Image* skybox;

        //Xi for the antialiasing
        unsigned short Xi[3] = {0, 0, 0};

        Scene(Camera camera, Image* skybox);
        Scene(std::vector<Object*> objects, std::vector<Light*> lights, Camera camera, Image* skybox);

        Image Render();

        Color rayCastColor(Point3 origin, Vector3 direction, int depth);   
        bool rayCastShadow(Light* light, Point3 lightOrigin, Vector3 normal, Point3 hitPoint);

        void addObject(Object* object);
        void addLight(Light* light);
        Color skyBox(Vector3 direction);

        //Utility functions
        bool Scatter(Vector3& r_in, Object *closestObject, Point3 hitPoint, Vector3 normal, Color& attenuation, Vector3& scattered, Color& outLightE);
        Color getReflectionColor(Point3 hitPoint, Vector3 direction, Object* closestObject, int depth);
        Color getRefractionColor(Point3 hitPoint, Vector3 direction, Object* closestObject, int depth, float ior); 
        Color getIndirectLighting(Point3 hitPoint, Vector3 normal, int depth);
        Color getDirectLighting(Point3 hitPoint, Vector3 normal, int depth, Object* closestObject, Vector3 direction);

        void getClosestObject(Point3 origin, Vector3 direction, float &minDistance, Object* &closestObject);
        void getClosestObject(Point3 origin, Vector3 direction, float &minDistance, Object* &closestObject, Object* exclude);
        Vector3 getRandomDirectionHemisphere(Vector3 normal);

        Color diffuseBRDF(Color diffuseColor, Vector3 lightDirection, Vector3 normal);
        Color specularBRDF(Color specularColor, Vector3 lightDirection, Vector3 viewDirection, Vector3 normal, float shine);

        Vector3 getRandomDirection();
        Vector3 reflect(Vector3 incident, Vector3 normal);
        bool refract(Vector3 incident, Vector3 normal, float ior, Vector3 &refracted);
        float clamp(float x, float lower, float upper);
        float schlick(float cosine, float ref_idx);

        bool loadOBJ(const std::string& filename, Point3 center, float scale);
    };


#endif