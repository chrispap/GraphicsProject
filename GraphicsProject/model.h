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

class Model
{
	vector<Point> mVertices;
	vector<Triangle> mTriangles;
	vector<set<int> > mVertexTriangles;
	Box mBox;

	void createBoundingBox ();
	void createTriangleLists ();
	void updateTriangleData ();
	void drawTriangles (bool wire=0);
	void drawNormals ();

	static void loadTrianglesFromOBJ (string filename, vector<Point> &vertices, vector<Triangle> &triangles, bool ccw, bool vt=0);
	static void findCollisions (const Model &m1, const Model &m2, vector<Point> &vertices, vector<Triangle> &triangles, bool both=0);

public:
	Model (string filename, bool ccw=0, bool vt=0);
	Model (const Model &m1, const Model &m2, bool intersectBoth=0);
	~Model (void);

	Box &getBox (){ return mBox;};
	void alingCornerToOrigin ();
	void alingCenterToOrigin ();
	void setSize (float size);
    void reduce (int lod=1);
	void draw (int x);
};

#endif
