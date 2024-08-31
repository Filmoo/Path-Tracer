#ifndef UTILS_HH
#define UTILS_HH

#include <cstdint>
#include "vector3.hh"
#include <cmath>
#include <algorithm>
extern uint32_t seed;

double random_double(uint32_t& state);
double random_double(uint32_t& state, double min, double max);  

Vector3 getRandomDirectionHemisphere(Vector3 normal);
Vector3 getRandomDirection();

Vector3 reflect(Vector3 incident, Vector3 normal);
bool refract(Vector3 incident, Vector3 normal, float nint, Vector3& refracted);

float schlick(float cosine, float ref_idx);

float clamp(float x, float lower, float upper);

#endif 