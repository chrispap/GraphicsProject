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
    static const int N;
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

    void loadScene();
    void drawScene();
    void intersectScene();
    void drawAxes();

    vector<Mesh*> armadillo;
    vector<Mesh*> car;
    vector<Mesh*> intersection;

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

    void keyEvent (unsigned char key, bool up=false, int x=0, int y=0 );
    void arrowEvent (int dir, int modif=0);
    void mousePressed (int x, int y, int modif=0);
    void mouseMoved (int x, int y, int modif=0);
    void mouseWheel (int dir=0, int modif=0);
};

#endif
