/** @file glvisuals.h
 * Definition of class GlVisuals.
 */

#ifndef VISUALS_H
#define VISUALS_H

#include "mesh.h"

static const Point globRot0(30,180,0);
static const Point globTrans0(0,0,0);

/**
 * Class that handles the scene and the user interface.
 */
class GlVisuals
{
    /* Parameters */
    bool perspective_proj;              ///< Orthographic or perspective projection
    float scene_size;                   ///< Arbitrary value used as basis for all sizes in the scene
    float scene_dist;                   ///< Distance of the scene from the camera
    Point globTrans, globRot;           ///< Position in the world | Local Rotation

    /* For UI */
    int mouselastX, mouselastY;         ///< Coords of mouse last click
    int mouseCurrX, mouseCurrY;         ///< Coords of mouse movement
    int screen_width, screen_height;    ///< Size of the windows in pixels
    int sel_i, sel_obj;                 ///< Selected objects
    int style, bvlStyle;                ///< The global style used for model drawing

    /* For animation */
    float t;                            ///< Elapsed time in seconds since the start of the animation
    int milli0;                         ///< Elapsed time in millieconds since the start of the animation

    /* Methods */
    void enterPixelMode ();             ///< Sets the matrices of openGL for 2D pixel space rendering
    void returnFromPixelMode ();        ///< Restores the normal 3D operation

    /* Scene objects */
    vector<Mesh*> armadillo;
    vector<Mesh*> car;
    vector<Mesh*> intersection;

    /* Manipulation of scene */
    void drawAxes ();
    void loadScene ();
    void drawScene ();
    void resetScene ();
    void intersectScene ();
    void simplifyObject (bool duplicate=false);

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
