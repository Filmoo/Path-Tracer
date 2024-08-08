#include "vector3.hh"
#include <math.h>

using Point3 = Vector3;

Vector3::Vector3() : x(0), y(0), z(0) {}
Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
Vector3::Vector3(Point3 from, Point3 to) : x(to.x - from.x), y(to.y - from.y), z(to.z - from.z) {}
Vector3 Vector3::operator*(const float &l) const
{
    return Vector3(x * l, y * l, z * l);
}
Vector3 Vector3::operator*(const Vector3 &v) const
{
    return Vector3(x * v.x, y * v.y, z * v.z);
}
Vector3 Vector3::operator+(const Vector3 &v) const
{
    return Vector3(x + v.x, y + v.y, z + v.z);
}
Vector3 Vector3::operator+(const float &f) const
{
    return Vector3(x + f, y + f, z + f);
}
Vector3 Vector3::operator-(const Vector3 &v) const
{
    return Vector3(x - v.x, y - v.y, z - v.z);
}
Vector3 Vector3::operator-(const float &f) const
{
    return Vector3(x - f, y - f, z - f);
}
Vector3 Vector3::operator/(const float &f) const
{
    return Vector3(x / f, y / f, z / f);
}
Vector3 Vector3::operator/(const Vector3 &v)
{
    return Vector3(x / v.x, y / v.y, z / v.z);
}
Vector3 Vector3::operator^(const Vector3 &v)
{
    return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}
bool Vector3::operator==(const Vector3 &v)
{
    return x == v.x && y == v.y && z == v.z;
}

float Vector3::dot(const Vector3 &v) const
{
    return x * v.x + y * v.y + z * v.z;
}
float Vector3::dot(const float &x) const
{
    return this->x * x + this->y * y + this->z * z;
}
Vector3 Vector3::cross(const Vector3 &v)
{
    return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}

Vector3 Vector3::normalize()
{
    float length = sqrt(x * x + y * y + z * z);
    if (length == 0)
        return Vector3(0, 0, 0);
    float invLength = 1/length;
    x *= invLength, y *= invLength, z *= invLength;
    return *this;
}
float Vector3::length() const
{
    return sqrt(x * x + y * y + z * z);
}
float Vector3::magnitude() const
{
    return sqrt(x * x + y * y + z * z);
}  
Vector3 Vector3::random(float min, float max)
{
    return Vector3(rand() % (int)max + min, rand() % (int)max + min, rand() % (int)max + min);
}



