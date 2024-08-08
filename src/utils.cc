#include "utils.hh"

uint32_t seed = 0x9e3779b9;

uint32_t PCG_Hash(uint32_t x) {
    uint32_t state = x * 747796405u + 2891336453u;
    uint32_t hash = ((state >>  ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return hash ^ (hash >> 22u);
}
double random_double(uint32_t& state) {
    state = PCG_Hash(state);
    return float(state) / float(UINT32_MAX);
}

double random_double(uint32_t& state, double min, double max) {
    return min + (max - min) * random_double(state);
}

Vector3 getRandomDirectionHemisphere(Vector3 normal)
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

    return randomDirection.normalize();
}