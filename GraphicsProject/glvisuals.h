#ifndef VISUALS_H
#define VISUALS_H

#include "mesh.h"

const float PI=3.14159;

class GlVisuals
{
    /* Parameters */
    float scene_size;
    float scene_dist;
    Point globalRot;
    Point globalTranslation;
    bool perspective_proj;

    /* For animation */
    float t;
    int milli0;

    /* For UI */
    int mouselastX;
    int mouselastY;
    int selObj;
    int selT;
    int style;

    /* Scene objects */
    vector<Mesh*> armadillo;
    vector<Mesh*> car;
    vector<Mesh*> intersection;

    /* Methods */
    void loadScene();
    void drawScene();
	void duplicateModel(bool shift);
    void intersectScene();
    void drawAxes();

public:
    GlVisuals();
    ~GlVisuals();
    void glInitialize();
    void glResize(int width, int height);
    void glPaint();

    void setEllapsedMillis (int milliseconds);
    void setGlobalRotation (const Point &rotVec) {globalRot = rotVec;}
    void setGlobalTranslation (const Point &tVec) {globalTranslation = tVec;}
    const Point &getGlobalRotation () {return globalRot;}
    const Point &getGlobalTranslation () {return globalTranslation;}

    void keyEvent (unsigned char key, bool up=false, int x=0, int y=0, int modif=0);
    void arrowEvent (int dir, int modif=0);
    void mousePressed (int x, int y, int modif=0);
    void mouseMoved (int x, int y, int modif=0);
    void mouseWheel (int dir=0, int modif=0);
};

enum ArrowDir {
    UP=0,
    DOWN,
    RIGHT,
    LEFT
};

#endif
