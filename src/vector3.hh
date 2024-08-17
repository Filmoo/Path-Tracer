#ifndef VECTOR_HH
#define VECTOR_HH

#include <iostream>

//Define class for vectors in 3D
class Vector3 {
    public:
        float x;
        float y;
        float z;
        Vector3();
        Vector3(float x, float y, float z);
        Vector3(Vector3 from, Vector3 to);
        //Methods for vector operations
        Vector3 operator*(const float &l) const;
        Vector3 operator*(const Vector3 &v) const;
        Vector3 operator+(const Vector3 &v) const;
        Vector3 operator-(const Vector3 &v) const;
        Vector3 operator+(const float &f) const;
        Vector3 operator-(const float &f) const;
        Vector3 operator/(const float &f) const;
        Vector3 operator/(const Vector3 &v);
        Vector3 operator^(const Vector3 &v);
        bool operator==(const Vector3 &v);
        float dot(const Vector3 &v) const;
        float dot(const float &x) const;
        Vector3 cross(const Vector3 &v);
        Vector3 normalize();
        Vector3 random(float min, float max);
        Vector3 orthogonal() const;
        float magnitude() const;
        float length() const;
        float length_squared() const;
        
        //Methods for displaying vectors in a good way
        friend std::ostream& operator<<(std::ostream &os, const Vector3& vec){
            os << "x: " << vec.x << " y: " << vec.y << " z: " << vec.z;
            return os;
        }
};

using Point3 = Vector3;

#endif