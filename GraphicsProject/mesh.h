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

using namespace std;

#define BVL_SIZE(L) ((1<<((L)+1))-1)    // MACRO giving the total number of nodes in a hierarchy tree with L levels
#define BVL 5                           // Number of levels of hierarchy of bounding volumes
#define VDIV 30                         // Number of divisions for volume scanning

class Mesh
{
    /** Data members */
    vector<Point> mVertices;            // Vertex list
    vector<Triangle> mTriangles;        // Triangle list | contains indices to the Vertex list
    vector<Point> mVertexNormals;       // Normals per vertex
    vector<set<int> > mVertexTriangles; // List of lists of the triangles that are connected to each vertex
    vector<list<int> > mAABBTriangles;  // Triangles of each AABB hierarchy level
    vector<list<int> > mSphereTriangles;// Triangles of each Sphere hierarchy level
    vector<Box> mAABB;                  // The bounding box hierarchy of the model
    vector<Sphere> mSphere;             // The bounding sphere hierarchy of the model
    vector<Box> mVoxels;                // The voxels that are generated during the volume calculation
    float coverage[BVL+1];              // Bounding box coverage of each hierarchy level
    Point mRot;                         // Model rotation around its local axis
    Point mPos;                         // Model position in the scene

    /** Private methods */
    void createTriangleLists ();        // Create lists with each vertex's triangles
    void createBoundingBoxHierarchy (); // Creates BVL levels of hierarchy by subdividing the boxes of each level
    void updateTriangleData ();         // Recalculates the plane equations of the triangles
    void calculateVolume ();            // Estimate the volume of the mesh by scanning all the bounding box
    void cornerAlign ();                // Align the mesh to the corner of each local axis
    void centerAlign ();                // Align the mesh to the center of each local axis
    void createNormals ();              // Create a normal for each vertex
    void hardTranslate (const Point &p);// Translation by adding the displacement to the vertices
    
    /** Drawing methods */
    void drawTriangles (Colour &col, bool wire=0);
    void drawAABB (Colour &col, bool hier=0);
    void drawTriangleBoxes (Colour &col);
    void drawNormals (Colour &col);
    void drawVoxels (Colour &col);

    /** Static methods */
    static void loadObj (string filename,   // Populate vertex | triangle lists from file
        vector<Point> &vertices, vector<Triangle> &triangles, bool ccw=0);
                                        
    static void findCollisions ( Mesh &m1,  Mesh &m2,   // Populate vertex | triangle lists with collisions of two other meshes */
        vector<Point> &vertices, vector<Triangle> &triangles, bool both=0); 

public:
    Mesh ();
    Mesh (string filename, bool ccw=0);         // Constructor from .obj file
    Mesh (Mesh &m1,  Mesh &m2, bool both=0);    // Constructor from intersection of other models
    Mesh (const Mesh &original);                // Copy constructor
   ~Mesh (void);                                // Destructor

    /** API */
    void draw (Colour &col, int style);         // Draw the mesh with the specified style
    void simplify (int percent=1);              // Try to reduce the number of faces preserving the shape
    void setMaxSize (float size);               // Set the meshes size according to the max size of three (x|y|z)
    void move (Point &p) { mPos.add(p);}        // Move the mesh in the world.
    void rotate (Point &p) { mRot.add(p);}      // Rotate mesh around its local axis
    void setPos (Point &p) { mPos = p;}         // Set the position of the mesh
    void setRot (Point &p) {mRot = p;}          // Set the rotation of the mesh
    const Box &getBox () { return mAABB[0];}    // Get the bounding box
    const Point &getPos() { return mPos;}       // Get the position
    const Point &getLocalRot() { return mRot;}  // Get the rotation
    float geCoverage(int l){return coverage[l];}// Get the percentage of coverage for a specific level of the box hierarchy
};

/**
 * Enum used to controll what to draw 
 * in a call of draw() 
 */
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
