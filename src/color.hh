#ifndef COLOR_HH
#define COLOR_HH
#include "vector3.hh"
#include <iostream>
// Define type for colors
struct Color {
    float r;
    float g;
    float b;

    // Constructor
    Color(float red, float green, float blue) : r(red), g(green), b(blue) {}
    Color() : r(0), g(0), b(0) {}
    Color(Vector3 v) : r(v.x), g(v.y), b(v.z) {}
    // Operator overloading
    friend std::ostream& operator<<(std::ostream &os, const Color& c){
        os << "r: " << (float) c.r << " g: " << (float)c.g << " b: " << (float)c.b;
        return os;
    }
    Color operator/(const float &l) const
    {
        return Color((float)(r / l), (float)(g / l), (float)(b / l));
    }
    Color operator/(const Color &c) const
    {
        return Color(r / c.r, g / c.g, b / c.b);
    }
    Color operator*(const float &l) const
    {
        return Color(r * l, g * l, b * l);
    }

    Color operator*(const Color &c) const
    {
        return Color(r * c.r, g * c.g, b * c.b);
    }

    Color operator+(const Color &c) const
    {
        return Color(r + c.r, g + c.g, b + c.g);
    }

    Color operator+(const float &l) const
    {
        return Color(r + l, g + l, b + l);
    }

    Color operator+=(const Color &c)
    {
        r += c.r;
        g += c.g;
        b += c.b;
        return *this;
    }

    Color operator+=(const float &l)
    {
        r += l;
        g += l;
        b += l;
        return *this;
    }
    Color operator+=(Vector3 &v)
    {
        r += v.x;
        g += v.y;
        b += v.z;
        return *this;
    }
    bool operator==(const Color &c) const
    {
        return (r == c.r) && (g == c.g) && (b == c.b);
    }
    
    bool operator!=(const Color &c) const
    {
        return !(*this==c);
    }

    Color Average(const Color& other) const {
        return Color((r + other.r) / 2.0f, (g + other.g) / 2.0f, (b + other.b) / 2.0f);
    }
    Color Clamp() const {
        return Color(std::min(1.0f, std::max(0.0f, r)), std::min(1.0f, std::max(0.0f, g)), std::min(1.0f, std::max(0.0f, b)));
    }
};



#endif
