#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include <set>
#include "geom.h"

#ifndef QT_CORE_LIB
#ifdef __linux__
#include <GL/glut.h>
#else
#include "gl/glut.h"
#endif
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

using namespace std;

#define SOLID		(1<<0)
#define WIRE		(1<<1)
#define NORMALS		(1<<2)
#define AABB		(1<<3)
#define TBOXES		(1<<4)

class Model
{
	Box mBox;							// The bounding box of the 3d model
	vector<Point> mVertices;			// Vertex list
	vector<Triangle> mTriangles;		// Triangle list | contains indices to the Vertex list
	vector<set<int> > mVertexTriangles;	// List of lists of the triangles that are connected to each vertex

	void createBoundingBox ();
	void createTriangleLists ();
	void updateTriangleData ();
	void drawTriangles (bool wire=0);
	void drawTriangleBoxes ();
	void drawNormals ();
	void drawAABB ();

	static void loadTrianglesFromOBJ (string filename, vector<Point> &vertices, vector<Triangle> &triangles, bool ccw, bool vt=0);
	static void findCollisions (const Model &m1, const Model &m2, vector<Point> &vertices, vector<Triangle> &triangles, bool both=0);

public:
	Model (string filename, bool ccw=0, bool vt=0);
	Model (const Model &m1, const Model &m2, bool intersectBoth=0);
	~Model (void);

	Box &getBox (){ return mBox;};
	float boxCoverage();
	void alingCornerToOrigin ();
	void alingCenterToOrigin ();
	void setSize (float size);
    void reduce (int LoD=1);
	void draw (int x);
};

#endif
