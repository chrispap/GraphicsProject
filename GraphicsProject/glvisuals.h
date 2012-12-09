#ifndef VISUALS_H
#define VISUALS_H

#include "model.h"

class GlVisuals
{
	static const float PI;
	bool perspective_proj;
    int xRot;
    int yRot;
    int zRot;
    int mouse_lastX;
    int mouse_lastY;
    float scene_size;
    float scene_dist;
    float scene_height;
    float t;
    int milli0;
	Model *m1;
	Model *m2;
	Model *m12;
    void drawAxes();
    
public:
    GlVisuals();
    ~GlVisuals();
    void glInitialize();
    void glResize(int width, int height);
    void glPaint();
    void mousePressed(int x, int y);
    void mouseMoved(int x, int y, int dir=0);
    void mouseWheel(int dir=0);
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);
    void setDistancePercent(int distance_percent);
    void setHeightPercent(int height_percent);
    void setEllapsedMillis(int milliseconds);
    int getXRotation(){return xRot;}
    int getYRotation(){return yRot;}
    int getZRotation(){return zRot;}
};

#endif
