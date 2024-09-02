#include "scene.hh"
#include <math.h>
#include <algorithm>
#include <thread>
#include <vector>
#include <iostream>
#include <future>
#include <atomic>
#include <limits>
#define M_PI  3.14159265358979323846
#define M_PIl 3.141592653589793238462643383279502884L
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

struct PathSegment {
    Point3 origin;
    Vector3 direction;
    Color color;
};
Scene::Scene(Camera* camera) 
    : objects(std::vector<Object*>()), lights(std::vector<Light*>()), camera(camera), skybox(nullptr) {}

Scene::Scene(Camera* camera, Image* skybox) 
    : objects(std::vector<Object*>()), lights(std::vector<Light*>()), camera(camera), skybox(skybox) {}

Scene::Scene(std::vector<Object*> objects, std::vector<Light*> lights, Camera* camera, Image* skybox)
    : objects(objects), lights(lights), camera(camera), skybox(skybox) {}


Image Scene::Render(int img_height, int img_width, int indirectSamples, int depth, int nb_threads, std::vector<Color>* accumulation_frame, int current_sample) {
    //Render an image depending on whats in the scene and where is the camera
    //We'll use a left-handed system of camera
    Vector3 C = *camera->C;
    img_height = (img_height < 1) ? 1 : img_height;

    Vector3 right = (*camera->forward).cross(*camera->up).normalize();
    Vector3 up = camera->up->normalize();

    img_height = (img_height < 1) ? 1 : img_height;

    Image img = Image(img_width, img_height);
    float focal_length = 1;
    float viewportHeight = 1;
    float viewportWidth = viewportHeight * (static_cast<float>(img_width) / img_height);

    Vector3 viewport_u = right * viewportWidth;
    Vector3 viewport_v = up * viewportHeight;
    Vector3 pixelDeltaU = viewport_u / img_width;
    Vector3 pixelDeltaV = viewport_v / img_height;
    Point3 viewportUpperLeft = C + *camera->forward * focal_length - viewport_u / 2.0f - viewport_v / 2.0f;
    const Point3 pixel00_loc =  viewportUpperLeft + (pixelDeltaU + pixelDeltaV) * 0.5;
    int maxThreads = std::thread::hardware_concurrency();
    if (nb_threads == 0)
        nb_threads = maxThreads;
        
    int numThreads = MIN(nb_threads, maxThreads);
	std::size_t max = img_width * img_height;
    std::atomic_size_t count(0);
    std::vector<std::future<void>> future_vector;
    std::vector<Color> pixels(img_width * img_height);
    while(numThreads--)
    {
        future_vector.emplace_back(
            std::async([=, &count, &img, &pixels, &accumulation_frame]()
            {
                while(true)
                {
                    std::size_t index = count++;
                    if (index >= max)
                    {
                        break;
                    }
                    int x = index % img.width;
                    int y = index / img.width;
                    Point3 pixel_loc = pixel00_loc + pixelDeltaU * x + pixelDeltaV * y;
                    Point3 pixel_sample = pixel_loc + (pixelDeltaU * (-0.5 + random_double(seed,0,1))) + (pixelDeltaV * (-0.5 + random_double(seed,0,1)));
                    Vector3 rayDirection = Vector3(C, pixel_sample).normalize();
                    Color sampleColor =  rayCastColor(C, rayDirection, depth);
                    accumulation_frame->at((img_height - y - 1) * img_width + x) += sampleColor;
                    pixels[y * img_width + x] = accumulation_frame->at(y * img_width + x) / current_sample;
                }
            }
            )
        );
    }
    for(auto &f : future_vector)
        f.get();

    Image output = Image(img_width, img_height);
    output.pixels = pixels;
    return output;
}


void Scene::getClosestObject(Point3 origin, Vector3 direction, float &minDistance, Object* &closestObject){
    for(Object* obj : objects)
    {
        float t1;
        if(obj->intersect(origin, direction, t1))
        {
            if (t1 < minDistance && t1 > 0.001f)
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
            if (t1 < minDistance && t1 > 0.001f)
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
            if (t1 < minDistance && t1 > 0.001f)
            {
                minDistance = t1;
                closestObject = obj;
            }
        }
    }
}


Color Scene::skyBox(Vector3 direction)
{
    if (skybox == nullptr)
        return Color(0,0,0);
    float u = 0.5f + atan2(direction.z, direction.x) / (2.0f * M_PI);
    float v = 0.5f - asin(direction.y) / M_PI;
    if (u < 0) u += 1.0f;
    if (v < 0) v += 1.0f;
    return skybox->getPixel(u * skybox->width, v * skybox->height);
}


Color Scene::rayCastColor(Point3 origin, Vector3 direction, int depth) {

    float minDistance = std::numeric_limits<float>::infinity();
    Object* closestObject = nullptr;
    getClosestObject(origin, direction, minDistance, closestObject);

    if (closestObject == nullptr || minDistance == std::numeric_limits<float>::infinity()) {
        return skyBox(direction);
    }

    Point3 hitPoint = origin + direction * minDistance;
    Vector3 normal = closestObject->normal(hitPoint).normalize();

    Vector3 scattered;
    Color attenuation;
    double pdf;
    Color emittedColor = closestObject->emission();
    
    if (!closestObject->texture->scatter(normal, direction, hitPoint, scattered, attenuation, pdf)) {
        return emittedColor;
    }

    //russian roulette
    
    auto prr = MAX(attenuation.r, MAX(attenuation.g, attenuation.b));

    if (depth > 5) {
        if (random_double(seed) > prr * 0.9) {
            return emittedColor;
        }
        attenuation = attenuation * (1.0f / prr);
    }
    
    float cosTheta = std::abs(normal.dot(scattered));
    Color castColor = rayCastColor(hitPoint, scattered, depth+1);
    return attenuation * castColor * cosTheta + emittedColor;
}


void Scene::addObject(Object* object){
    objects.push_back(object);
}

void Scene::addLight(Light* light){
    lights.push_back(light);
}
