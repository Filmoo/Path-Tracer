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
#define M_PIl 3.141592653589793238462643383279502884L
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
            : c(c), kd(kd), ks(ks), shine(shine), iorefrac(iorefrac), ioreflec(ioreflec){};
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
            double cosine = normal.dot(scattered);
            return cosine < 0 ? 0 : cosine / M_PIl;
        }
        bool scatter(Vector3 normal, Vector3 incident, Point3 hitPoint, Vector3 &scattered, Color &attenuation,  double& pdf) 
        {
            Vector3 target = hitPoint + normal + getRandomDirectionHemisphere(normal);
            scattered = (target - hitPoint).normalize();  
            attenuation = c;    
            pdf = scattering_pdf(normal, scattered);           
            return pdf != 0;
        }
};

class Metal_Texture : public Texture_Material {
    public:
        Color c;
        float roughness;
        Metal_Texture(Color c, float roughness)
            : c(c), roughness(roughness) {}
        void getTextureMaterial(Vector3 p, Color &c, float &kd, float &ks, float &shine, float &iorefrac, float &ioreflec)
        {
            c = this->c;
            kd = 0;
            ks = 1;
            shine = 32;
            iorefrac = 0;
            ioreflec = 1;
        }
        double scattering_pdf(Vector3 normal, Vector3 scattered) 
        { 
            double cosine = normal.dot(scattered);
            return cosine < 0 ? 0 : cosine / M_PIl;
        }
        bool scatter(Vector3 normal, Vector3 incident, Point3 hitPoint, Vector3 &scattered, Color &attenuation,  double& pdf) 
        {
            Vector3 reflected = reflect(incident, normal);
            scattered = reflected.normalize() + getRandomDirectionHemisphere(reflected) * roughness;
            attenuation = c;
            pdf = scattering_pdf(normal, scattered);
            return pdf != 0;
;
        }
};

class Dieletric_Texture : public Texture_Material {
    public:
        float iorefrac;
        Color c;
        Dieletric_Texture(Color c, float iorefrac)
            : c(c), iorefrac(iorefrac) {}
        void getTextureMaterial(Vector3 p, Color &c, float &kd, float &ks, float &shine, float &iorefrac, float &ioreflec)
        {
            c = c;
            kd = 0;
            ks = 0;
            shine = 0;
            iorefrac = this->iorefrac;
            ioreflec = 0;
        }
        double scattering_pdf(Vector3 normal, Vector3 scattered) 
        { 
            //pdf calculation for dielectric material
            return 1;
        }
        bool scatter(Vector3 normal, Vector3 incident, Point3 hitPoint, Vector3 &scattered, Color &attenuation, double& pdf) 
        {
            Vector3 outwardN;
            Vector3 reflected = reflect(incident, normal);
            Vector3 refracted;
            float ni_over_nt;
            attenuation = c;
            float reflect_prob;
            float cosine;
            if (incident.dot(normal) > 0)
            {
                outwardN = normal * -1;
                ni_over_nt = iorefrac;
                cosine = incident.normalize().dot(normal);
            }
            else
            {
                outwardN = normal;
                ni_over_nt = 1.0f / iorefrac;
                cosine = -incident.normalize().dot(normal);
            }
            reflect_prob = refract(incident, outwardN, ni_over_nt, refracted) ? schlick(cosine, iorefrac) : 1.0;
            scattered = random_double(seed) < reflect_prob ? reflected : refracted;
            pdf = scattering_pdf(normal, scattered);
            if (pdf == 0)
                return false;
            return true;
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