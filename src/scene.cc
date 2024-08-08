#include "scene.hh"
#include <math.h>
#include <algorithm>
#include <thread>
#include <vector>
#include <iostream>
#include <future>
#include <atomic>
#include "sphere.hh"
#define M_PI 3.14159265358979323846

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
                    img.setPixel(x,y,cumulativeColor / indirectSamples);
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


bool Scene::rayCastShadow(Light* light, Point3 lightOrigin, Vector3 normal, Point3 hitPoint) {
    Vector3 lightDirection = (lightOrigin - hitPoint);
    float lightDistance = lightDirection.length();
    lightDirection = lightDirection.normalize();
    float t1;   
    for (Object* obj : objects) {
        if (dynamic_cast<Light *>(obj) != nullptr)
            continue;
        if (obj->intersect(hitPoint + lightDirection * 1e-3f, lightDirection, t1) 
            && t1 < lightDistance) {
            return true;
        }
    }
    return false;
}

Color Scene::skyBox(Vector3 direction)
{
    float u = 0.5f + atan2(direction.z, direction.x) / (2.0f * M_PI);
    float v = 0.5f - asin(direction.y) / M_PI;
    if (u < 0) u += 1.0f;
    if (v < 0) v += 1.0f;
    return skybox->getPixel(u * skybox->width, v * skybox->height);
}

bool Scene::Scatter(Vector3& r_in, Object *closestObject, Point3 hitPoint, Vector3 normal, Color& attenuation, Vector3& scattered, Color& outLightE) {
    outLightE = Vector3(0, 0, 0);
    if (dynamic_cast<Uniform_Texture *>(closestObject->texture) != nullptr)
    {
        Vector3 target = hitPoint + normal + getRandomDirectionHemisphere(normal);
        scattered = (target - hitPoint).normalize();  
        Color albedo;
        float kd, ks, shine, iorefrac, ioreflec;
        closestObject->getTextureMaterial(hitPoint, albedo, kd, ks, shine, ioreflec, iorefrac);
        attenuation = albedo;
    }   
    else 
    if (dynamic_cast<Metal_Texture *>(closestObject->texture) != nullptr)
    {
        Metal_Texture* metal = dynamic_cast<Metal_Texture *>(closestObject->texture);
        Vector3 reflected = reflect(r_in, normal).normalize();
        scattered = ((reflected + getRandomDirectionHemisphere(normal) * metal->fuzz) - hitPoint).normalize();
        attenuation = metal->c;
        return scattered.dot(normal) > 0;
    }
    else
    if (dynamic_cast<Dieletric_Texture *>(closestObject->texture) != nullptr)
    {
        Dieletric_Texture* dieletric = dynamic_cast<Dieletric_Texture *>(closestObject->texture);
        Vector3 outwardN;
        Vector3 reflected = reflect(r_in, normal);
        Vector3 refracted;
        float ni_over_nt;
        attenuation = Vector3(1, 1, 1);
        float reflect_prob;
        float cosine;
        if (r_in.dot(normal) > 0)
        {
            outwardN = normal * -1;
            ni_over_nt = dieletric->iorefrac;
            cosine = ni_over_nt * r_in.dot(normal);
        }
        else
        {
            outwardN = normal;
            ni_over_nt = 1.0f / dieletric->iorefrac;
            cosine = -r_in.dot(normal);
        }
        if (refract(r_in, outwardN, ni_over_nt, refracted))
        {
            reflect_prob = schlick(cosine, dieletric->iorefrac);
        }
        else
        {
            reflect_prob = 1.0f;
        }
        if (random_double(seed) < reflect_prob)
        {
            scattered = reflected;
        }
        else
        {
            scattered = refracted;
        }
    }
    else
    if (dynamic_cast<Emitter_Texture *>(closestObject->texture) != nullptr)
    {
        Emitter_Texture* emitter = dynamic_cast<Emitter_Texture *>(closestObject->texture);
        attenuation = emitter->c;
        return false;
    }
    else
    {
        std::cerr << "No texture found\n";
        return false;
    }
    return true;
}

Color Scene::rayCastColor(Point3 origin, Vector3 direction, int depth) {

    float minDistance = std::numeric_limits<float>::max();
    Object* closestObject = nullptr;
    getClosestObject(origin, direction, minDistance, closestObject);

    if (closestObject == nullptr || minDistance == std::numeric_limits<float>::max()) {
        return Color(0,0,0);
        return skyBox(direction);
    }

    Point3 hitPoint = direction * minDistance + origin;
    Vector3 normal = closestObject->normal(hitPoint);
    if (normal.dot((hitPoint - origin).normalize()) > 0)
    {
        normal = normal * -1;
    }


    Vector3 scattered;
    Color attenuation;
    double pdf;
    Color emittedColor = closestObject->emission();
    Color outLightE;
    if (depth == 0 || !closestObject->texture->scatter(normal, direction, hitPoint, scattered, attenuation, pdf))
    {
        return emittedColor;
    }
    double scatttering_pdf = closestObject->texture->scattering_pdf(normal, scattered);
    Color castColor = rayCastColor(hitPoint, scattered, depth - 1);
    Color scatteredColor = (castColor * attenuation * scatttering_pdf)/pdf;
    return (scatteredColor + emittedColor).Clamp();

    /*
    //Russian roulette to terminate path that won't contribute significantly to the final image
    float continueProb = std::min(std::max(attenuation.r, std::max(attenuation.g, attenuation.b)), 1.0f);
    if (random_double(seed, 0, 1) < continueProb)
    {
        Color casted = rayCastColor(hitPoint, scattered, depth - 1);
        return (casted * attenuation + outLightE).Clamp() / continueProb;
    }
    else
    {
        return (emittedColor).Clamp();
    }
    */
}

Vector3 Scene::getRandomDirectionHemisphere(Vector3 normal)
{
    float r1 = random_double(seed);
    float r2 = random_double(seed);
    float phi = 2 * M_PI * r1;
    float x = cos(phi) * sqrt(1 - r2);
    float y = sin(phi) * sqrt(1 - r2);
    float z = sqrt(r2);

    Vector3 randomDirection(x, y, z);

    if (randomDirection.dot(normal) < 0.0) {
        randomDirection = randomDirection * (-1);
    }

    return randomDirection;
}


Vector3 Scene::getRandomDirection() {
    double z = 2.0 * random_double(seed) - 1.0;
    double t = 2.0 * M_PI * random_double(seed);
    double r = sqrt(1.0 - z * z);
    double x = r * cos(t);
    double y = r * sin(t);
    return Vector3(x, y, z).normalize();
}


Vector3 Scene::reflect(Vector3 incident, Vector3 normal) {
    return incident - normal * 2.0f * incident.dot(normal);
}

bool Scene::refract(Vector3 incident, Vector3 normal, float nint, Vector3& refracted) {
    float cosi = incident.dot(normal);
    float k = 1 - nint * nint * (1 - cosi * cosi);
    if (k < 0)
    {
        refracted = Vector3(0, 0, 0);
        return false;
    }
    else
    {
        refracted = (incident - normal * cosi) * nint - normal * sqrt(k);
        return true;    
    }
}

float Scene::clamp(float x, float lower, float upper) {
    return std::max(lower, std::min(x, upper));
}

void Scene::addObject(Object* object){
    objects.push_back(object);
}

void Scene::addLight(Light* light){
    lights.push_back(light);
}

float Scene::schlick(float cosine, float ref_idx) {
    float r0 = (1 - ref_idx) / (1 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}

bool Scene::loadOBJ(const std::string& filename, Point3 center, float scale)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str());

    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "ERR: " << err << std::endl;
    }

    if (!ret) {
        return false;
    }

    // Process the shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = shapes[s].mesh.num_face_vertices[f];

            std::vector<Point3> vertices;
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];

                vx = vx * scale + center.x;
                vy = vy * scale + center.y;
                vz = vz * scale + center.z;

                vertices.push_back(Point3(vx, vy, vz));
            }
            index_offset += fv;

            if (fv == 3) {
                int material_id = shapes[s].mesh.material_ids[f];
                tinyobj::material_t material = materials[material_id];
                
                std::cout << "Material: " << material.name << std::endl;
                std::cout << "Diffuse: " << material.diffuse[0] << " " << material.diffuse[1] << " " << material.diffuse[2] << std::endl;
                std::cout << "Specular: " << material.specular[0] << " " << material.specular[1] << " " << material.specular[2] << std::endl;
                std::cout << "Shininess: " << material.shininess << std::endl;
                std::cout << "Ior: " << material.ior << std::endl;
                Texture_Material* texture = new Dieletric_Texture(material.ior);
                //Texture_Material* texture = new Uniform_Texture(Color(0.5,0.5,0.5), 4, 1, 32, 0, 0);
                // Add the triangle to the scene
                
                /* std::cout << "triangle\n";
                std::cout << vertices[0].x << " " << vertices[0].y << " " << vertices[0].z << std::endl;
                std::cout << vertices[1].x << " " << vertices[1].y << " " << vertices[1].z << std::endl;
                std::cout << vertices[2].x << " " << vertices[2].y << " " << vertices[2].z << std::endl;
                  */
                Triangle* triangle = new Triangle(texture, vertices[0], vertices[1], vertices[2]);
                addObject(triangle);
            }
            else {
                std::cerr << "Only triangles are supported" << std::endl;
            }
        }
    }

    return true;
}
