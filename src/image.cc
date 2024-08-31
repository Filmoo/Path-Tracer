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

ID3D11ShaderResourceView* Image::CreateTextureFromImage(ID3D11Device* g_pd3dDevice)
{
    std::lock_guard<std::mutex> lock(pixelMutex);
    std::vector<float> imageData(width * height * 4);
    for (int i = 0; i < width * height; ++i) {
        pixels[i] = pixels[i].Clamp();
    }
    for (int i = 0; i < width * height; ++i) {
        imageData[4 * i] = 0.0f;
        imageData[4 * i + 1] = 0.0f;
        imageData[4 * i + 2] = 0.0f;
        imageData[4 * i + 3] = 1.0f;
    }
    for (int i = 0; i < width * height; ++i) {
        imageData[4 * i] = pixels[i].r;
        imageData[4 * i + 1] = pixels[i].g;
        imageData[4 * i + 2] = pixels[i].b;
        imageData[4 * i + 3] = 1.0f; 
    }
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    ID3D11Texture2D* pTexture = nullptr;
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = imageData.data();
    initData.SysMemPitch = width * 4 * sizeof(float); // Width * RGBA * sizeof(float)
    HRESULT hr = g_pd3dDevice->CreateTexture2D(&desc, &initData, &pTexture);
    if (FAILED(hr)) {
        std::cerr << "Failed to create texture. HRESULT: " << hr << std::endl;
        return nullptr;
    }
    ID3D11ShaderResourceView* pTextureView = nullptr;
    if (pTexture)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        hr = g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &pTextureView);
        if (FAILED(hr)) {
            std::cerr << "Failed to create shader resource view. HRESULT: " << hr << std::endl;
            pTexture->Release();
            return nullptr;
        }
    }
    if (pTexture)
        pTexture->Release();
    
    return pTextureView;
}