#include "image.hh"


void Image::save(const char *filename)
{
    FILE *f = fopen(filename, "w");
    fprintf(f, "P3\n%d\n%d\n%d\n", width, height, 255);
    for(int y = 0; y < height; y++)
        for(int x = 0; x < width; x++)
        {
            Color c = getPixel(x,y);
            fprintf(f,"%d %d %d\n", (int)round(c.r * 255), (int)round(c.g * 255), (int)round(c.b * 255));
        }
}

void Image::setPixel(int x, int y, Color c)
{
    if (x >= 0 && x < width && y >= 0 && y < height) {
        pixels[y * width + x] = c;
    } else {
        std::cerr << "Error: setPixel called with out-of-bounds coordinates (" << x << ", " << y << ")\n";
    }
}

Color Image::getPixel(int x, int y)
{
    if (x >= 0 && x < width && y >= 0 && y < height) {
        return pixels[y * width + x];
    } else {
        std::cerr << "Error: getPixel called with out-of-bounds coordinates (" << x << ", " << y << ")\n";
        return Color(0, 0, 0); 
    }
}
Image::Image(Image &image) {
    width = image.width;
    height = image.height;
    pixels = image.pixels;
}
Image::Image(int width, int height)
{
    this->width = width;
    this->height = height;
    // We create a matrix with row-major order
    pixels = std::vector<Color>(width * height);
}

Image::Image(const char *filename) {
    std::ifstream file(filename, std::ios::binary);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return;
    }

    // Read the first line to determine format
    std::getline(file, line);
    if (line != "P6") {
        std::cerr << "Error: Unsupported PPM file format" << std::endl;
        return;
    }

    // Read the second line which should be the width and height
    std::getline(file, line);
    std::istringstream dimensions(line);
    dimensions >> width >> height;

    // Create pixels array
    pixels = std::vector<Color>(width * height);

    // Read the maximum color value
    std::getline(file, line); // Assuming it's 255 for now

    // Read pixel data
    unsigned char *buffer = new unsigned char[3 * width * height];
    file.read(reinterpret_cast<char *>(buffer), 3 * width * height);

    for (int i = 0; i < width * height; ++i) {
        pixels[i] = {
            buffer[3 * i] / 255.0f,
            buffer[3 * i + 1] / 255.0f,
            buffer[3 * i + 2] / 255.0f
        };
    }

    delete[] buffer;

    file.close();
}

std::vector<float> Image::data() {
    std::vector<float> data(width * height * 3);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            Color c = getPixel(x, y);
            int idx = (y * width + x) * 3;
            data[idx] = c.r;
            data[idx + 1] = c.g;
            data[idx + 2] = c.b;
        }
    }
    return data;
}
float* Image::dataPointer() {
    std::vector<float> floatData = data();
    float* buffer = new float[floatData.size()];
    std::copy(floatData.begin(), floatData.end(), buffer);
    return buffer;
}

void Image::toLinear() {
    auto srgbToLinear = [](float c) -> float {
        if (c <= 0.04045f)
            return c / 12.92f;
        else
            return pow((c + 0.055f) / 1.055f, 2.4f);
    };

    for (auto& pixel : pixels) {
        pixel.r = srgbToLinear(pixel.r);
        pixel.g = srgbToLinear(pixel.g);
        pixel.b = srgbToLinear(pixel.b);
    }
}

void Image::toRGB() {
    auto linearToSrgb = [](float c) -> float {
        if (c <= 0.0031308f)
            return 12.92f * c;
        else
            return 1.055f * pow(c, 1.0f / 2.4f) - 0.055f;
    };

    for (auto& pixel : pixels) {
        pixel.r = linearToSrgb(pixel.r);
        pixel.g = linearToSrgb(pixel.g);
        pixel.b = linearToSrgb(pixel.b);
    }
}
