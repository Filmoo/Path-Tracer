#include "sphere.hh"
#include "plane.hh"
#include "box.hh"
#include "camera.hh"
#include "light.hh"
#include "scene.hh"
#include "blob.hh"
#include "obj_reader.hh"
#include "triangle.hh"

#include <chrono>
#include <iostream>


int main()
{   
    try {
    Color Red = Color(0.5, 0.5, 0.5);
    Color Blue = Color(0, 0, 0.5);

    Point3* centerCam = new Point3(2, 0, 2);
    Point3* focusCam = new Point3(1, 0, 0);
    Vector3* upCam = new Vector3(1, 0, 0);
    Camera* camera = new Camera(*centerCam, *focusCam, *upCam, 20, 20, 1, 1 , 2, (float)16/9);

    Scene* scene = new Scene(*camera, new Image("../textures/sky.ppm"));
    
   
    Plane *ground = new Plane(new Uniform_Texture(Color(0.4,0.4,0.4), 0.9, 0.5, 32, 0, 0), Point3(0,0,0), Vector3(0,0,1));
    scene->addObject(ground); 
    
    Plane *back = new Plane(new Uniform_Texture(Color(0.2,0.2,0.2), 0.9, 0.5, 32, 0, 0), Point3(15,0,0), Vector3(-1,0,0));
    scene->addObject(back); 
    Plane *left = new Plane(new Uniform_Texture(Color(0,0,1), 0.9, 0.5, 32, 0, 0), Point3(0,5,0), Vector3(0,-1,0));
    scene->addObject(left);
    Plane *right = new Plane(new Uniform_Texture(Color(1,0,0), 0.9, 0.5, 32, 0, 0), Point3(0,-5,0), Vector3(0,1,0));
    scene->addObject(right);
    Plane *top = new Plane(new Uniform_Texture(Color(0.2,0.2,0.2), 0.9, 0.5, 32, 0, 0), Point3(5,0,5), Vector3(0,0,-1));
    scene->addObject(top);
    Plane *behind = new Plane(new Uniform_Texture(Color(0, 0, 0.7), 0.9, 0.5, 32, 0, 0), Point3(0, 0, 0), Vector3(-1,0,0));
    scene->addObject(behind);   
    
    
    Sphere *glassSphere = new Sphere(new Dieletric_Texture(Color(1,1,1),1.2), Point3(8, 1.5, 1), 1);
    //scene->addObject(glassSphere); 
    Sphere *glassSphere2 = new Sphere(new Dieletric_Texture(Color(1,1,1),2.5), Point3(8, -3, 1), 1);
    //scene->addObject(glassSphere2);   
    
    Sphere *normalSphere1 = new Sphere(new Uniform_Texture(Color(0,1,0), 0.5, 0.5, 32, 0, 0), Point3(10, -2, 1), 1);
    scene->addObject(normalSphere1);
    Sphere *normalSphere2 = new Sphere(new Uniform_Texture(Color(1,0,0), 0.9, 1, 32, 0, 0), Point3(10, 2, 1), 1);
    scene->addObject(normalSphere2);

    
    Sphere *metalSphere1 = new Sphere(new Metal_Texture(Color(1,1,1), 0.1), Point3(10, 0, 2), 1);
    scene->addObject(metalSphere1);

    Sphere_Light *lightSphere = new Sphere_Light(Color(1,1,1), Point3(10, 0, 104.7), 100, 0.6);
    //Sphere_Light *lightSphere = new Sphere_Light(Color(1,1,1), Point3(10, 0, 4), 1, 1);

    scene->addObject(lightSphere);
    scene->addLight(lightSphere);


    //scene->loadOBJ("models/Glass_Cup.obj", Point3(5,0,2), 0.5);


    std::cout << "Number of objects in the scene: " << scene->objects.size() << std::endl;
    std::cout << "Rendering scene" << std::endl;
    std::cout << "Number of samples for indirect lighting: " << scene->indirectSamples << std::endl;
    std::cout << "Depth of the ray casting: " << scene->depth << std::endl;
    auto start = std::chrono::high_resolution_clock::now(); 
    Image image = scene->Render();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Rendering time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
    image.save("Image.ppm");

    }   
    catch (const char* msg) {
        std::cerr << msg << std::endl;
        std::cout << "Rendering failed" << std::endl;
    }
    return 0;
}




