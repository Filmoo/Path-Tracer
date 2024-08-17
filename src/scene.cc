#include "scene.hh"
#include <math.h>
#include <algorithm>
#include <thread>
#include <vector>
#include <iostream>
#include <future>
#include <atomic>
#define M_PI  3.14159265358979323846
#define M_PIl 3.141592653589793238462643383279502884L

struct PathSegment {
    Point3 origin;
    Vector3 direction;
    Color color;
};

Scene::Scene(Camera camera, Image* skybox) 
    : objects(std::vector<Object*>()), lights(std::vector<Light*>()), camera(camera), skybox(skybox) {}

Scene::Scene(std::vector<Object*> objects, std::vector<Light*> lights, Camera camera, Image* skybox)
    : objects(objects), lights(lights), camera(camera), skybox(skybox) {}


Image Scene::Render(){
    //Render an image depending on whats in the scene and where is the camera
    //We'll use a left-handed system of camera
    const Point3 C = camera.C;
    Vector3 camDirection = camera.forward;
    camDirection = camDirection.normalize();

    int img_width = 1000;
    int img_height = static_cast<int>(img_width / camera.focalLength);  
    std::cout << "Resolution: " << img_width << "x" << img_height << "\n";
    img_height = (img_height < 1) ? 1 : img_height;
    Image img = Image(img_width, img_height);
    float focal_length = 1;
    float viewportHeight = 1;
    float viewportWidth = viewportHeight * (static_cast<float>(img_width) / img_height);
    Vector3 viewport_u = Vector3(0, viewportWidth, 0);
    Vector3 viewport_v = Vector3(0 ,0 ,-viewportHeight);
    Vector3 pixelDeltaU = viewport_u / img_width;
    Vector3 pixelDeltaV = viewport_v / img_height;
    Point3 viewportUpperLeft = C - viewport_u / 2 - viewport_v / 2 + Vector3(focal_length,0,0);
    const Point3 pixel00_loc =  viewportUpperLeft + (pixelDeltaU + pixelDeltaV) * 0.5;
    int numThreads = std::thread::hardware_concurrency();
    //numThreads = 1;
	std::size_t max = img.width * img.height;
    std::atomic_size_t count(0);
    std::vector<std::future<void>> future_vector;
    std::cout << "Number of threads: " << numThreads << "\n";
    while(numThreads--)
    {
        future_vector.emplace_back(
            std::async([=, &count, &img]()
            {
                while(true)
                {
                    std::size_t index = count++;
                    if (index >= max)
                        break;
                    int x = index % img.width;
                    int y = index / img.width;
                    Color cumulativeColor(0, 0, 0);
                    for( int s = 0; s < indirectSamples; s++)
                    {   
                        Point3 pixel_loc = pixel00_loc + pixelDeltaU * x + pixelDeltaV * y;
                        Point3 pixel_sample = pixel_loc + (pixelDeltaU * (-0.5 + random_double(seed,0,1))) + (pixelDeltaV * (-0.5 + random_double(seed,0,1)));
                        Vector3 rayDirection = Vector3(C, pixel_sample).normalize();
                        cumulativeColor +=  rayCastColor(C, rayDirection, depth);
                    }
                    cumulativeColor = cumulativeColor * (1.0f / indirectSamples);
                    img.setPixel(x,y,cumulativeColor);
                }
            }
            )
        );
    }
    return img;
}


void Scene::getClosestObject(Point3 origin, Vector3 direction, float &minDistance, Object* &closestObject){
    for(Object* obj : objects)
    {
        float t1;
        if(obj->intersect(origin, direction, t1))
        {
            if (t1 < minDistance)
            {
                minDistance = t1;
                closestObject = obj;
            }
        }
    }
}
void Scene::getClosestObject(Point3 origin, Vector3 direction, float &minDistance, Object* &closestObject, Object* exclude){
    for(Object* obj : objects)
    {
        if (dynamic_cast<Light*>(obj) != nullptr)
            continue;
        float t1;
        if(obj != exclude && obj->intersect(origin, direction, t1))
        {
            if (t1 < minDistance)
            {
                minDistance = t1;
                closestObject = obj;
            }
        }
    }
}
void Scene::getClosestObject(Point3 origin, Vector3 direction, float &minDistance, Object* &closestObject, Object* exclude1, Object* exclude2){
    for(Object* obj : objects)
    {
        float t1;
        if (dynamic_cast<Light*>(obj) != nullptr)
            continue;
        if(obj != exclude1 && obj != exclude2 && obj->intersect(origin, direction, t1))
        {
            if (t1 < minDistance)
            {
                minDistance = t1;
                closestObject = obj;
            }
        }
    }
}


Color Scene::skyBox(Vector3 direction)
{
    float u = 0.5f + atan2(direction.z, direction.x) / (2.0f * M_PI);
    float v = 0.5f - asin(direction.y) / M_PI;
    if (u < 0) u += 1.0f;
    if (v < 0) v += 1.0f;
    return skybox->getPixel(u * skybox->width, v * skybox->height);
}


Color Scene::rayCastColor(Point3 origin, Vector3 direction, int depth) {
    if (depth > 10) {
        return Color(0, 0, 0);
    }
    float minDistance = std::numeric_limits<float>::max();
    Object* closestObject = nullptr;
    getClosestObject(origin, direction, minDistance, closestObject);

    if (closestObject == nullptr || minDistance == std::numeric_limits<float>::max()) {
        return skyBox(direction);
    }

    Point3 hitPoint = direction * minDistance + origin;
    Vector3 normal = closestObject->normal(hitPoint);
    normal = normal.dot(direction) < 0 ? normal : normal * -1;

    Vector3 scattered;
    Color attenuation;
    double pdf;
    Color emittedColor = closestObject->emission();
    
    if (!closestObject->texture->scatter(normal, direction, hitPoint, scattered, attenuation, pdf)) {
        return emittedColor;
    }
    float prr = std::max(attenuation.r, std::max(attenuation.g, attenuation.b));
    //russian roulette
    if (depth > 5 && random_double(seed,0,1) > prr)
            return emittedColor;
    scattered = scattered.normalize();
    float cosTheta = normal.dot(scattered);
    Color castColor = rayCastColor(hitPoint, scattered, depth+1);
    return attenuation * castColor * cosTheta / (prr) + emittedColor;
}


void Scene::addObject(Object* object){
    objects.push_back(object);
}

void Scene::addLight(Light* light){
    lights.push_back(light);
}
