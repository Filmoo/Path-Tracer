#ifndef OBJ_READER_HH
#define OBJ_READER_HH

#include "mesh.hh"
#include "triangle.hh"

#include <vector>
#include <fstream>
#include <sstream>
#include <string>

class ObjReader {
    public:
        static Mesh readObj(std::string filename) {
            Mesh mesh;
            //Read the file and get triangles and textures
            
            return mesh; 
        }
};

#endif