#ifndef VISUALS_H
#define VISUALS_H

#include "mesh.h"

enum ArrowDir {
	UP=0,
	DOWN,
	RIGHT,
	LEFT
};

class GlVisuals
{
	static const float PI;
	bool perspective_proj;
	Point globalRot;
	Point globalTranslation;

	float scene_size;
	float scene_dist;
	float t;
	int milli0;
	int mouselastX;
	int mouselastY;
	int selObj;
	int selT;

	void loadModels();
	void intersectModels();
	void drawAxes();
	void drawScene();

	Mesh *armadillo[5];
	Mesh *car[5];
	Mesh *intersection[5];

public:
	GlVisuals();
	~GlVisuals();
	void glInitialize();
	void glResize(int width, int height);
	void glPaint();

	void setEllapsedMillis (int milliseconds);
	void setGlobalRotation (const Point &rotVec) {globalRot = rotVec;};
	void setGlobalTranslation (const Point &tVec) {globalTranslation = tVec;};
	Point getGlobalRotation () {return globalRot;};
	Point getGlobalTranslation () {return globalTranslation;};

	void keyEvent (unsigned char key, int x, int y, bool up);
	void arrowEvent (int dir, int modif=0);
	void mousePressed (int x, int y, int modif=0);
	void mouseMoved (int x, int y, int modif=0);
	void mouseWheel (int dir=0, int modif=0);
};

#endif
