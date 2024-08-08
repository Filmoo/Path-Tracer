#ifndef BLOB_HH
#define BLOB_HH

#include <vector>
#include "vector3.hh"
#include "mesh.hh"
#include "blob_tables.hh"

class Blob
{
    public:
        float e;
        float d;
        float isosurface_level;
        std::vector<Point3> points;
        float potential(Point3 p)
        {
            float potential = 0;
            for(auto pp : points){
                potential += 1 / (p - pp).length();
            }
            return potential;
        }
        void addPoint(Point3 p)
        {
            points.push_back(p);
        }
        Blob(float e, float d, float isosurface_level)
        : e(e), d(d), isosurface_level(isosurface_level){}

        Mesh generateMesh(bool smoothen = true) {
            Mesh mesh; 
            Texture_Material* texture = new Uniform_Texture(Color(1, 1, 1), 0, 0, 0, 0, 0);   
            //Generate triangles using marching cubes algorithm
            for(float x = -e ; x < e; x += d)
            {
                for(float y = -e; y < e; y += d)
                {
                    for(float z = -e; z < e; z += d)
                    {
                        Point3 p1 = Point3(x, y, z);
                        Point3 p4 = Point3(x + d, y, z);
                        Point3 p3 = Point3(x + d, y + d, z);
                        Point3 p2 = Point3(x, y + d, z);
                        Point3 p5 = Point3(x, y, z + d);
                        Point3 p8 = Point3(x + d, y, z + d);
                        Point3 p7 = Point3(x + d, y + d, z + d);
                        Point3 p6 = Point3(x, y + d, z + d);
                        
                        float potential1 = potential(p1);
                        float potential2 = potential(p2);
                        float potential3 = potential(p3);
                        float potential4 = potential(p4);
                        float potential5 = potential(p5);
                        float potential6 = potential(p6);
                        float potential7 = potential(p7);
                        float potential8 = potential(p8);

                        Point3 arretes[12] = {
                            (p1 + p2)/2, (p2 + p3)/2,
                            (p3 + p4)/2, (p4 + p1)/2,
                            (p5 + p6)/2, (p6 + p7)/2, 
                            (p7 + p8)/2, (p8 + p5)/2, 
                            (p5 + p1)/2, (p6 + p2)/2, 
                            (p7 + p3)/2, (p8 + p4)/2};

                        
                        Point3 arretesInterpolatedLinearPotential[12] = {
                            p1 + (p2 - p1) * (isosurface_level - potential1) / (potential2 - potential1),
                            p2 + (p3 - p2) * (isosurface_level - potential2) / (potential3 - potential2),
                            p3 + (p4 - p3) * (isosurface_level - potential3) / (potential4 - potential3),
                            p4 + (p1 - p4) * (isosurface_level - potential4) / (potential1 - potential4),
                            p5 + (p6 - p5) * (isosurface_level - potential5) / (potential6 - potential5),
                            p6 + (p7 - p6) * (isosurface_level - potential6) / (potential7 - potential6),
                            p7 + (p8 - p7) * (isosurface_level - potential7) / (potential8 - potential7),
                            p8 + (p5 - p8) * (isosurface_level - potential8) / (potential5 - potential8),
                            p5 + (p1 - p5) * (isosurface_level - potential5) / (potential1 - potential5),
                            p6 + (p2 - p6) * (isosurface_level - potential6) / (potential2 - potential6),
                            p7 + (p3 - p7) * (isosurface_level - potential7) / (potential3 - potential7),
                            p8 + (p4 - p8) * (isosurface_level - potential8) / (potential4 - potential8)};
                        

                        
                        
                        int cubeindex = 0;
                        if (potential1 < isosurface_level) cubeindex |= 1;
                        if (potential2 < isosurface_level) cubeindex |= 2;
                        if (potential3 < isosurface_level) cubeindex |= 4;
                        if (potential4 < isosurface_level) cubeindex |= 8;
                        if (potential5 < isosurface_level) cubeindex |= 16;
                        if (potential6 < isosurface_level) cubeindex |= 32;
                        if (potential7 < isosurface_level) cubeindex |= 64;
                        if (potential8 < isosurface_level) cubeindex |= 128;

                        for(int i = 0; triTable[cubeindex][i] != -1; i += 3)
                        {
                            if (!smoothen) 
                            {
                                Point3 point1 = arretes[triTable[cubeindex][i]];
                                Point3 point2 = arretes[triTable[cubeindex][i + 1]];
                                Point3 point3 = arretes[triTable[cubeindex][i + 2]]; 
                                Triangle t = Triangle(texture, point1, point2, point3);
                                mesh.addTriangle(t);
                            }
                            else 
                            {
                                Point3 point1 = arretesInterpolatedLinearPotential[triTable[cubeindex][i]];
                                Point3 point2 = arretesInterpolatedLinearPotential[triTable[cubeindex][i + 1]];
                                Point3 point3 = arretesInterpolatedLinearPotential[triTable[cubeindex][i + 2]];
                                //Calculate the normals of each corner of the triangles
                                Vector3 normal1 = Vector3(potential(point1 + Vector3(0.01, 0, 0)) - potential(point1 - Vector3(0.01, 0, 0)),
                                                        potential(point1 + Vector3(0, 0.01, 0)) - potential(point1 - Vector3(0, 0.01, 0)),
                                                        potential(point1 + Vector3(0, 0, 0.01)) - potential(point1 - Vector3(0, 0, 0.01))).normalize();
                                Vector3 normal2 = Vector3(potential(point2 + Vector3(0.01, 0, 0)) - potential(point2 - Vector3(0.01, 0, 0)),
                                                        potential(point2 + Vector3(0, 0.01, 0)) - potential(point2 - Vector3(0, 0.01, 0)),
                                                        potential(point2 + Vector3(0, 0, 0.01)) - potential(point2 - Vector3(0, 0, 0.01))).normalize();
                                Vector3 normal3 = Vector3(potential(point3 + Vector3(0.01, 0, 0)) - potential(point3 - Vector3(0.01, 0, 0)),
                                                        potential(point3 + Vector3(0, 0.01, 0)) - potential(point3 - Vector3(0, 0.01, 0)),
                                                        potential(point3 + Vector3(0, 0, 0.01)) - potential(point3 - Vector3(0, 0, 0.01))).normalize();
                                Triangle t = Triangle(texture, point1, point2, point3, normal1, normal2, normal3);
                                mesh.addTriangle(t);
                            }
                        }
                    }
                }
            }
            



            return mesh;
        }

};

#endif 