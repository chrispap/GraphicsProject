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

#define BVL			4
#define SOLID		(1<<0)
#define WIRE		(1<<1)
#define NORMALS		(1<<2)
#define AABB		(1<<3)
#define TBOXES		(1<<4)

class Mesh
{
	vector<Point> mVertices;			// Vertex list
	vector<Triangle> mTriangles;		// Triangle list | contains indices to the Vertex list
	vector<set<int> > mVertexTriangles;	// List of lists of the triangles that are connected to each vertex
	Point localRot, localTranslation;	// Transformations
	vector<list<int> > mAABBTriangles;	// Triangles of each hierarchy level
	vector<Box> mAABB;					// The bounding box hierarchy of the 3d model
	float coverage;

	void createBoundingBox ();
	void createTriangleLists ();
	void updateTriangleData ();
	void calculateVolume();
	void drawTriangles (const Colour &col, bool wire=0);
    void drawTriangleBoxes (const Colour &col);
    void drawNormals (const Colour &col);
    void drawAABB (const Colour &col);

	static void loadTrianglesFromOBJ (string filename, vector<Point> &vertices, vector<Triangle> &triangles, bool ccw, bool vt=0);
	static void findCollisions (const Mesh &m1, const Mesh &m2, vector<Point> &vertices, vector<Triangle> &triangles, bool both=0);

public:
	Mesh ();
	Mesh (string filename, bool ccw=0, bool vt=0);
	Mesh (const Mesh &m1, const Mesh &m2, bool intersectBoth=0);
	Mesh (const Mesh &original);
	~Mesh (void);

	void alignLocalCorner ();
	void alignLocalCenter ();
	void setSize (float size);
	void translate (const Point &p);
	void setLocalTranslation (const Point &p);
	void reduce (int LoD=1);
	void draw (const Colour &col, int x);

    const Point &getLocalTranslation() const { return localTranslation;}
    const Point &getLocalRotation() const { return localRot;}
    float getBoundingCoverage() const { return coverage;}
    const Box &getBox () const { return mAABB[0];}
};

#endif
