#ifndef VISUALS_H
#define VISUALS_H

#include "mesh.h"

class GlVisuals
{
	static const float PI;
	bool perspective_proj;
    Point globalRot, globalTranslation;
     
    int mouse_lastX, mouse_lastY;
    int selObj, selT;
    float scene_size, scene_dist;
    float t;
    int milli0;
    void drawAxes();
    void drawScene();
    void makeIntersections();
    
	Mesh *armadillo[5];
	Mesh *car[5];
	Mesh *intersection[5];
    
public:
    GlVisuals();
    ~GlVisuals();
    void glInitialize();
    void glResize(int width, int height);
    void glPaint();
    
    void mousePressed(int x, int y, int modif=0);
    void mouseMoved(int x, int y, int modif=0);
    void mouseWheel(int dir=0, int modif=0);
	void keyEvent (unsigned char key, int x, int y, bool up);
	void arrowEvent (int dir, int modif=0);
    
    void setGlobalXRotation(int angle);
    void setGlobalYRotation(int angle);
    void setGlobalZRotation(int angle);
    void setGlobalXTranslation(int d);
    void setGlobalYTranslation(int d);
    void setGlobalZTranslation(int d);
    
    void setEllapsedMillis(int milliseconds);
    
};

#endif
