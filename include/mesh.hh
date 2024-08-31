#ifndef MESH_HH
#define MESH_HH

#include "scene.hh"
#include "triangle.hh"
#include "texture.hh"

#include <vector>

class Mesh {
    public:
        std::vector<Triangle> triangles;
        
        Mesh(std::vector<Triangle> triangles) 
        : triangles(triangles) {}

        Mesh() {}

        void addToScene(Scene &scene) {
            for (int i = 0; i < triangles.size(); i++) {
                scene.addObject(&triangles[i]);
            }
        }

        void addTriangle(Triangle t) {
            triangles.push_back(t);
        }
        void addTriangles(std::vector<Triangle> t) {
            for (int i = 0; i < t.size(); i++) {
                triangles.push_back(t[i]);
            }
        }
        void addMesh(Mesh m) {
            if (m.triangles.size() == 0) {
                return;
            }
            for (int i = 0; i < m.triangles.size(); i++) {
                triangles.push_back(m.triangles[i]);
            }
        }
};

#endif