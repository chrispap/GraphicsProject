#ifndef VISUALS_H
#define VISUALS_H

#include "mesh.h"

const float PI=3.14159;
const Point globRot0(30,180,0);
const Point globTrans0(0,0,0);

class GlVisuals
{
    /* Parameters */
    float scene_size;
    float scene_dist;
    Point globRot;
    Point globTrans;
    bool perspective_proj;

    /* For animation */
    float t;
    int milli0;

    /* For UI */
    int mouselastX;
    int mouselastY;
    int sel_i;
    int sel_obj;
    int style;

    /* Scene objects */
    vector<Mesh*> armadillo;
    vector<Mesh*> car;
    vector<Mesh*> intersection;

    /* Methods */
    void loadScene();
    void drawScene();
    void resetScene();
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
    void setGlobalRotation (const Point &rotVec) {globRot = rotVec;}
    void setGlobalTranslation (const Point &tVec) {globTrans = tVec;}
    const Point &getGlobalRotation () {return globRot;}
    const Point &getGlobalTranslation () {return globTrans;}

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
