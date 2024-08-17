#include "utils.hh"

uint32_t seed = 0x9e3779b9;

uint32_t PCG_Hash(uint32_t x) {
    uint32_t state = x * 747796405u + 2891336453u;
    uint32_t hash = ((state >>  ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return hash ^ (hash >> 22u);
}
double random_double(uint32_t& state) {
    state = PCG_Hash(state);
    return static_cast<double>(state) / static_cast<double>(UINT32_MAX);
}

double random_double(uint32_t& state, double min, double max) {
    return min + (max - min) * random_double(state);
}


Vector3 getRandomDirectionHemisphere(Vector3 normal)
{
    float r1 = random_double(seed);
    float r2 = random_double(seed);
    float sinTheta = sqrt(1 - r1 * r1);
    float phi = 2 * M_PI * r2;
    return Vector3(sinTheta * cos(phi), sinTheta * sin(phi), r1).normalize();
}


Vector3 getRandomDirection() {
    double z = 2.0 * random_double(seed) - 1.0;
    double t = 2.0 * M_PI * random_double(seed);
    double r = sqrt(1.0 - z * z);
    double x = r * cos(t);
    double y = r * sin(t);
    return Vector3(x, y, z).normalize();
}


Vector3 reflect(Vector3 incident, Vector3 normal) {
    incident = incident.normalize();
    normal = normal.normalize();
    return incident - normal * 2.0f * incident.dot(normal);
}

bool refract(Vector3 incident, Vector3 normal, float nint, Vector3& refracted) {
    incident = incident.normalize();
    normal = normal.normalize();
    float cosi = incident.dot(normal);
    float k = 1 - nint * nint * (1 - cosi * cosi);
    if (k < 0)
    {
        refracted = Vector3(0, 0, 0);
        return false;
    }
    else
    {
        refracted = incident * nint + normal * (nint * cosi - sqrt(k));
        return true;    
    }
}

float schlick(float cosine, float ref_idx) {
    float r0 = (1 - ref_idx) / (1 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}

float clamp(float x, float lower, float upper) {
    return std::max(lower, std::min(x, upper));
}
