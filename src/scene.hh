#ifndef SCENE_HH
#define SCENE_HH

#include "vector3.hh"
#include "object.hh"
#include "light.hh" 
#include "camera.hh"
#include "utils.hh"
#include "tiny_obj_loader.h"
#include "triangle.hh"
#include "sphere.hh"
#include "plane.hh" 

#include <vector>
#include <limits>

class Scene {
    public:
        std::vector<Object*> objects;

        std::vector<Light*> lights;

        Camera camera;

        //Maximum depth of the ray casting
        int depth = 0;

        //Number of samples for indirect lighting
        int indirectSamples = 100;
        //Skybox
        Image* skybox;

        Scene(Camera camera);
        Scene(Camera camera, Image* skybox);
        Scene(std::vector<Object*> objects, std::vector<Light*> lights, Camera camera, Image* skybox);

        Image Render();
        Color rayCastColor(Point3 origin, Vector3 direction, int depth);   


        Color skyBox(Vector3 direction);

        void addObject(Object* object);
        void addLight(Light* light);
        void getClosestObject(Point3 origin, Vector3 direction, float &minDistance, Object* &closestObject);
        void getClosestObject(Point3 origin, Vector3 direction, float &minDistance, Object* &closestObject, Object* exclude);
        void getClosestObject(Point3 origin, Vector3 direction, float &minDistance, Object* &closestObject, Object* exclude1, Object* exclude2);

    };


#endif