#ifndef TEXTURE_HH
#define TEXTURE_HH

#include "vector3.hh"
#include "color.hh"
#include "image.hh"
#include "utils.hh"
#include <vector>
#include <algorithm>
#include <cmath>
#include <fstream>

enum Material_Type {
    Lambertian,
    Metal,
    Dieletric,
    Emitter,
};

class Texture_Material {
    public:
        //Method that, given position on the surface, 
        //returns elements of the texture and material at that point.
        //      - kd is the diffuse coefficient
        //      - ks is the specular coefficient
        //      - shine is the shininess coefficient
        Color c;
        float kd;
        float ks;
        float shine;
        float iorefrac;
        float ioreflec;
        virtual void getTextureMaterial(Vector3 p, Color &c, float &kd, float &ks, float &shine, float &iorefrac, float &ioreflec) = 0;
        virtual double scattering_pdf(Vector3 normal, Vector3 scattered) { return 0; }
        virtual bool scatter(Vector3 normal, Vector3 incident, Point3 hitPoint, Vector3 &scattered, Color &attenuation, double &pdf) { return false; }
};

class Uniform_Texture : public Texture_Material {
    public:
        Color c;
        float kd;
        float ks;
        float shine;
        float iorefrac;
        float ioreflec;
        Uniform_Texture(Color c, float kd, float ks, float shine, float iorefrac, float ioreflec)
            : c(c), kd(kd), ks(ks), shine(shine), iorefrac(iorefrac), ioreflec(ioreflec) {};
        void getTextureMaterial(Vector3 p, Color &c, float &kd, float &ks, float &shine, float &iorefrac, float &ioreflec)
        {
            c = this->c;
            kd = this->kd;
            ks = this->ks;
            shine = this->shine;
            iorefrac = this->iorefrac;
            ioreflec = this->ioreflec;
        }
        double scattering_pdf(Vector3 normal, Vector3 scattered) 
        { 
            auto cosine = normal.dot(scattered);
            return cosine < 0 ? 0 : cosine / M_PI;
        }
        bool scatter(Vector3 normal, Vector3 incident, Point3 hitPoint, Vector3 &scattered, Color &attenuation, double& pdf) 
        {
            Vector3 target = hitPoint + normal + getRandomDirectionHemisphere(normal);
            scattered = (target - hitPoint).normalize();  
            attenuation = c;    
            pdf = scattering_pdf(normal, scattered);
            return true;
        }
};

class Metal_Texture : public Texture_Material {
    public:
        Color c;
        float fuzz;
        Metal_Texture(Color c, float fuzz)
            : c(c), fuzz(fuzz) {}
        void getTextureMaterial(Vector3 p, Color &c, float &kd, float &ks, float &shine, float &iorefrac, float &ioreflec)
        {
            c = this->c;
            kd = 0;
            ks = 1;
            shine = 32;
            iorefrac = 0;
            ioreflec = 1;
        }
        bool scatter(Vector3 normal, Vector3 incident, Point3 hitPoint, Vector3 &scattered, Color &attenuation, double& pdf) 
        {
            Vector3 target = hitPoint + normal + getRandomDirectionHemisphere(normal);
            scattered = (target - hitPoint).normalize();  
            attenuation = c;    
            pdf = scattering_pdf(normal, scattered);
            return true;
        }
};

class Dieletric_Texture : public Texture_Material {
    public:
        float iorefrac;
        Dieletric_Texture(float iorefrac)
            : iorefrac(iorefrac) {}
        void getTextureMaterial(Vector3 p, Color &c, float &kd, float &ks, float &shine, float &iorefrac, float &ioreflec)
        {
            c = Color(0.5,0.5,0.5);
            kd = 0;
            ks = 0;
            shine = 0;
            iorefrac = this->iorefrac;
            ioreflec = 0;
        }
        double scattering_pdf(Vector3 normal, Vector3 scattered) 
        { 
            auto cosine = normal.dot(scattered);
            return cosine < 0 ? 0 : cosine / M_PI;
        }
};

class Emitter_Texture : public Texture_Material {
    public:
        Color c;

        Emitter_Texture(Color c)
            : c(c) {}
        void getTextureMaterial(Vector3 p, Color &c, float &kd, float &ks, float &shine, float &iorefrac, float &ioreflec)
        {
            c = this->c;
            kd = 0;
            ks = 0;
            shine = 0;
            iorefrac = 0;
            ioreflec = 0;
        }
};

class Sphere_Texture : public Texture_Material {
    public:
        Image* image;
        float kd;
        float ks;
        float shine;
        float iorefrac;
        float ioreflec;
        Sphere_Texture(const char* ppm, float kd, float ks, float shine, float iorefrac, float ioreflec)
            : kd(kd), ks(ks), shine(shine), iorefrac(iorefrac), ioreflec(ioreflec) 
            {
                image = new Image(ppm);
                if (image == nullptr) {
                    std::cerr << "Error: could not load image " << ppm << std::endl;
                    exit(1);
                }
            }
        void getTextureMaterial(Vector3 p, Color &c, float &kd, float &ks, float &shine, float &iorefrac, float &ioreflec)
        { 
            //Mapping the point on the sphere to the texture
            float u = 0.5 + atan2(-p.x, p.y) / (2 * M_PI);
            float v = 0.5 - asin(p.z) / M_PI;   
            c = sampleTexture(u,v);

            kd = this->kd;
            ks = this->ks;
            shine = this->shine;
            iorefrac = this->iorefrac;
            ioreflec = this->ioreflec;
        }
    private:
        Color sampleTexture(float u, float v)
        {
            int x = static_cast<int>(u * (image->width));
            int y = static_cast<int>(v * (image->height));
            return image->getPixel(x,y);
        }
};



#endif