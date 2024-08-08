#ifndef IMAGE_HH
#define IMAGE_HH

#include "color.hh"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <fstream>
#include <sstream>

class Image {
    public:
        int width;
        int height;
        std::vector<Color> pixels;
        Image(int width, int height);
        Image(const char* filename);

        void setPixel(int x, int y, Color c);
        Color getPixel(int x, int y);

        //Method to save a ppm image
        void save(const char *filename);
        std::vector<float> data();
        float* dataPointer();
        void toLinear();
        void toRGB();
};





#endif