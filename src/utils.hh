#ifndef UTILS_HH
#define UTILS_HH

#include <cstdint>
#include "vector3.hh"
#include <cmath>

extern uint32_t seed;

double random_double(uint32_t& state);

double random_double(uint32_t& state, double min, double max);  

Vector3 getRandomDirectionHemisphere(Vector3 normal);

#endif 