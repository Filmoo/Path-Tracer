#ifndef IMAGE_HH
#define IMAGE_HH

#include "color.hh"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <mutex>

class Image {
    public:
        int width;
        int height;
        std::vector<Color> pixels;
        std::mutex pixelMutex;
        Image(Image &image);
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

        //operator=
        Image& operator=(const Image &image)
        {
            width = image.width;
            height = image.height;
            // We create a matrix with row-major order
            pixels =  std::vector<Color>(image.width * image.height);
            for (int i = 0; i < image.width * image.height; i++)
            {
                pixels[i] = image.pixels[i];
            }
            return *this;
        }
};





#endif