/** @file mesh.h
 * Definition of class Mesh
 */

#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <list>
#include <set>
#include "geom.h"

#ifdef __linux__
#include <GL/glut.h>
#else
#include "gl/glut.h"
#endif

using namespace std;

#define BVL_SIZE(L) ((1<<((L)+1))-1)            ///< MACRO giving the total number of nodes in a hierarchy tree with L levels
#define BVL     7                               ///< Number of levels of hierarchy of bounding volumes
#define VDIV    50                              ///< Number of divisions for volume scanning

/**
 * Class that handles a model.
 */
class Mesh
{
    /** Data members */
    vector<Point> mVertices;                    ///< Vertex list
    vector<Triangle> mTriangles;                ///< Triangle list | contains indices to the Vertex list
    vector<Point> mVertexNormals;               ///< Normals per vertex
    vector<set<int> > mVertexTriangles;         ///< List of lists of the triangles that are connected to each vertex
    vector<list<int> > mAABBTriangles;          ///< Triangles of each AABB hierarchy level
    vector<list<int > > mSphereTriangles;       ///< Triangles of each Sphere hierarchy level
    vector<Box> mAABB;                          ///< The bounding box hierarchy of the model
    vector<Sphere> mSphere;                     ///< The bounding sphere hierarchy of the model
    vector<Box> mVoxels;                        ///< The voxels that are generated during the volume calculation
    float AABBCover[BVL+1];                     ///< Bounding box coverage of each hierarchy level
    float sphereCover[BVL+1];                   ///< Bounding sphere coverage of each hierarchy level
    Point mRot;                                 ///< Model rotation around its local axis
    Point mPos;                                 ///< Model position in the scene

    /** Private methods */
    void createTriangleLists ();                ///< Create lists with each vertex's triangles
    void createBoundingVolHierarchy ();         ///< Creates BVL levels of hierarchy of bounding volumes
    void createBoundingBoxHierarchy ();         ///< Creates BVL levels of hierarchy of bounding boxes
    void createBoundingSphereHierarchy ();      ///< Creates BVL levels of hierarchy of bounding spheres
    void updateTriangleData ();                 ///< Recalculates the plane equations of the triangles
    void calculateVolume ();                    ///< Estimate the volume of the mesh by scanning all the bounding box
    void cornerAlign ();                        ///< Align the mesh to the corner of each local axis
    void centerAlign ();                        ///< Align the mesh to the center of each local axis
    void createNormals ();                      ///< Create a normal for each vertex
    void hardTranslate (const Point &p);        ///< Translation by adding the displacement to the vertices

    void drawTriangles (Colour col,bool wire=0);///< Draw the triangles. This is the actual model drawing.
    void drawSphere (Colour col, bool hier=0);  ///< Draw the boundig sphere of the model
    void drawAABB (Colour col, bool hier=0);    ///< Draw the bounding box of the object
    void drawTriangleBoxes (Colour col);        ///< Draw the bounding boxes of each triangle
    void drawNormals (Colour col);              ///< Draw the normal vectors of each vertex
    void drawVoxels (Colour col);               ///< Draw the cubes that were used to calculate the volume of the object

    static void loadObj (string filename,       ///< Populate vertex | triangle lists from file
        vector<Point> &vertices, vector<Triangle> &triangles, bool ccw=0);

    static void intersect (Mesh &m1,  Mesh &m2, ///< Populate vertex | triangle lists with collisions of two other meshes */
        vector<Point> &vertices, vector<Triangle> &triangles, bool both=0);

public:
    Mesh ();
    Mesh (string filename, bool ccw=0);         ///< Constructor from .obj file
    Mesh (Mesh &m1,  Mesh &m2, bool both=0);    ///< Constructor from intersection of other models
    Mesh (const Mesh &original);                ///< Copy constructor
   ~Mesh (void);                                ///< Destructor

    void draw (Colour col, int style);          ///< Draw the mesh with the specified style
    void simplify (int percent=1);              ///< Try to reduce the number of faces preserving the shape
    void setMaxSize (float size);               ///< Set the meshes size according to the max size of three (x|y|z)
    void move (Point &p) { mPos.add(p);}        ///< Move the mesh in the world.
    void rotate (Point &p) { mRot.add(p);}      ///< Rotate mesh around its local axis
    void setPos (Point &p) { mPos = p;}         ///< Set the position of the mesh
    void setRot (Point &p) {mRot = p;}          ///< Set the rotation of the mesh
    const Box &getBox () { return mAABB[0];}    ///< Get the bounding box
    const Point &getPos() { return mPos;}       ///< Get the position
    const Point &getLocalRot() { return mRot;}  ///< Get the rotation
    float getAABBCoverage(int l) { return AABBCover[l];}        ///< Get the percentage of coverage for a specific level of the box hierarchy
    float getSphereCoverage(int l) { return sphereCover[l];}    ///< Get the percentage of coverage for a specific level of the sphere hierarchy

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
    SPHERE  = (1<<4),
    HIER    = (1<<5),
    TBOXES  = (1<<6),
    VOXELS  = (1<<7)
};

#endif
