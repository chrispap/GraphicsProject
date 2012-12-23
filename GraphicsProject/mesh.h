#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <list>
#include <set>
#include "geom.h"

#ifndef QT_CORE_LIB
#ifdef __linux__
#include <GL/glut.h>
#else
#include "gl/glut.h"
#endif
#else
#include <GL/glu.h>
#endif

#define BVL_SIZE(L) ((1<<((L)+1))-1)
#define BVL 5
#define VDIV 20

using namespace std;

class Mesh
{
    /** Data members */
    vector<Point> mVertices;            // Vertex list
    vector<Triangle> mTriangles;        // Triangle list | contains indices to the Vertex list
    vector<Point> mVertexNormals;       // Normals per vertex
    vector<set<int> > mVertexTriangles; // List of lists of the triangles that are connected to each vertex
    vector<list<int> > mAABBTriangles;  // Triangles of each hierarchy level
    vector<Box> mAABB;                  // The bounding box hierarchy of the model
    vector<Box> mVoxels;                // The voxels that are generated during the volume calculation
    float coverage[BVL+1];              // Bounding box coverage of each hierarchy level
    Point mRot;                         // Model rotation around its local axis
    Point mPos;                         // Model position in the scene

    void createBoundingBoxHierarchy ();
    void createTriangleLists ();
    void createNormals ();
    void updateTriangleData ();
    void calculateVolume ();
    void hardTranslate(const Point &p);
    void drawTriangles (const Colour &col, bool wire=0);
    void drawTriangleBoxes (const Colour &col);
    void drawNormals (const Colour &col);
    void drawAABB (const Colour &col, bool skipHier=false);
    void drawVoxels (const Colour &col);

    /** Static utility methodos */
    static void loadTrianglesFromOBJ (string filename,
        vector<Point> &vertices, vector<Triangle> &triangles, bool ccw);

    static void findCollisions ( Mesh &m1,  Mesh &m2,
        vector<Point> &vertices, vector<Triangle> &triangles, bool both=0);

public:
    Mesh ();
    Mesh (string filename, bool ccw=0);
    Mesh ( Mesh &m1,  Mesh &m2, bool intersectBoth=0);
    Mesh (const Mesh &original);
   ~Mesh (void);

    /** API */
    void reduce (int LoD=1);
    void draw (const Colour &col, int x);
    void cornerAlign ();
    void centerAlign ();
    void setMaxSize (float size);
    void move (const Point &p) { mPos.add(p);}
    void rotate (const Point &p) { mRot.add(p);}
    void setPosition (const Point &p) { mPos = p;}
    void setRotation (const Point &p) {mRot = p;}
    const Box &getBox () const { return mAABB[0];}
    const Point &getPosition() const { return mPos;}
    const Point &getLocalRotation() const { return mRot;}
    float geCoverage(int level) const { return coverage[level];}
};

enum Style {
    SOLID   = (1<<0),
    WIRE    = (1<<1),
    NORMALS = (1<<2),
    AABB    = (1<<3),
    AABBH   = (1<<4),
    TBOXES  = (1<<5),
    VOXELS  = (1<<6)
};

#endif
