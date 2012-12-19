#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include "glvisuals.h"
#include "mesh.h"

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

GlVisuals::GlVisuals():
    globalTranslation (0,0,0),
    globalRot (30,180,0),
    perspective_proj (1),
    scene_size (100),
    scene_dist (scene_size*0.8),
    selObj (-1),
    selT ('z'),
    milli0 (-1),
    t (0.0),
    style (SOLID)
{
    loadScene();
}

GlVisuals::~GlVisuals()
{
    for (int i=0; i<armadillo.size(); ++i)
        delete armadillo[i];

    for (int i=0; i<car.size(); ++i)
        delete car[i];

    for (int i=0; i<intersection.size(); ++i)
        delete intersection[i];
}

/** Manage scene */
void GlVisuals::loadScene()
{

    /****************
     * Load Model 1 *
     ****************/
    puts("\n ==== Armadillo ====");
    armadillo.push_back (new Mesh("Model_1.obj", 0, 0));
    armadillo[0]->setMaxSize(scene_size/2);

    /****************
     * Load Model 2 *
     ****************/
    puts("\n ==== Car ====");
    car.push_back (new Mesh("Model_2.obj", 1, 0));
    car[0]->setMaxSize(scene_size/3);

}

void GlVisuals::intersectScene()
{
    if (selObj<0 || (selObj+1)>armadillo.size())
        cout << "Select a valid object first" << endl;
    else {
        cout << " * Intersections * " << endl;

        for (int i=0; i<intersection.size(); ++i)
            delete intersection[i];

        intersection.clear();

        for (int i=0; i<car.size(); ++i) {
            intersection.push_back (new Mesh(*armadillo[selObj], *car[i], 1));
        }

        for (int i=0; i<armadillo.size(); ++i) {
            if (i==selObj) continue;
            intersection.push_back (new Mesh(*armadillo[selObj], *armadillo[i], 1));
        }

    }

}

void GlVisuals::drawScene()
{
    for (int i=0; i<armadillo.size(); ++i)
        armadillo[i]->draw (Colour(0x66,0x66,0), style | ((i==selObj)?AABB:0));

    for (int i=0; i<car.size(); ++i)
        car[i]->draw (Colour(0,0x66,0x66), style | ((i==selObj)?AABB:0));

    for (int i=0; i<intersection.size(); ++i)
        intersection[i]->draw (Colour(0x66,0,0x66), SOLID | WIRE);
}


/** OpenGL Callback Methods */
void GlVisuals::glInitialize()
{
    static GLfloat ambientLight[] = { 1.0, 1.0, 1.0, 1.0 };
    static GLfloat diffuseLight[] = { 1.0, 1.0, 1.0, 1.0 };
    static GLfloat lightPos[] = { 0.0, 0.0, -scene_size, 0.0 };

    glLightfv( GL_LIGHT0, GL_AMBIENT, ambientLight );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuseLight );
    glLightfv( GL_LIGHT0, GL_POSITION, lightPos );

    glShadeModel( GL_FLAT );
    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc( GL_LEQUAL );
    glClearDepth(1.0);

    //glEnable(GL_CULL_FACE);
    glEnable(GL_POINT_SMOOTH);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0);
    glClearColor(0.2, 0.2, 0.2, 1);
}

void GlVisuals::glResize(int w, int h)
{
    if(!perspective_proj) {
        int ww,hh;
        if (h==0) hh=1;
        if (w==0) ww=1;
        glViewport(0,0,w,h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        if (h>w) glOrtho(-scene_size/2, scene_size/2, -scene_size/2*h/w, scene_size/2*h/w, -10.0f*scene_size/2, 10.0f*scene_size/2);
    } else {
        if (h==0) h=1;
        glViewport(0,0,w,h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = (float)w/(float)h;    // aspect ratio
        gluPerspective(60.0, aspect, 1.0, 100.0*scene_size);
    }
}

void GlVisuals::glPaint()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Push away the scene */
    glTranslatef(0,0,-scene_dist);

    /* Apply global transformations */
    glTranslatef(globalTranslation.x, globalTranslation.y, globalTranslation.z);
    glRotatef(globalRot.x, 1, 0, 0);
    glRotatef(globalRot.y, 0, 1, 0);
    glRotatef(globalRot.z, 0, 0, 1);
    drawAxes();
    drawScene();
}

void GlVisuals::drawAxes()
{
    glBegin(GL_LINES);
    //[X]
    glColor3ub(0xFF, 0, 0);
    glVertex2f(0.0,0.0);
    glVertex2f(10.0*scene_size,0.0);
    //[Y]
    glColor3f(0, 0xFF, 0);
    glVertex2f(0.0,0.0);
    glVertex2f(0.0,10.0*scene_size);
    //[Z]
    glColor3f(0, 0, 0xFF);
    glVertex2f(0.0,0.0);
    glVertex3f(0.0,0.0,10.0*scene_size);

    glEnd();
}


/** UI Methods */
void GlVisuals::setEllapsedMillis(int millis)
{
    // In the first call calibrate.
    if (milli0<0)
        milli0 = millis;
    else
        t = ((millis-milli0)/1000.0);
}

void GlVisuals::keyEvent (unsigned char key,  bool up, int x, int y, int modif)
{
    key = tolower(key);
    int _style=0;

    if (up) {
        //selObj = -1;
    }
    else {
        if (key>='x' && key <='z') selT = key;
        else if (key>='1' && key <= '9') selObj = key-'0'-1;
        else if (key == '0') selObj = -1;
        else if (key=='i') intersectScene();
        else if (key=='s') _style = SOLID;
        else if (key=='w') _style = WIRE;
        else if (key=='v') _style = VOXELS;
        else if (key=='b') _style = AABBH;

        if (_style) style = (style&_style)? (style&(~_style)): style|_style;
    }

}

void GlVisuals::arrowEvent (int dir, int modif)
{
    bool ctrl  =  modif & 0x01;
    bool shift =  modif & 0x02;

    if (selObj<0)
    {
        Point &t = globalTranslation;
        float &e = ctrl? t.z : dir&2? t.x : t.y;
        e = dir&1? e - scene_size/20: e + scene_size/20;
    }
    else
    {
        Point t(0,0,0);
        float &e = ctrl? t.y : dir&2? t.z : t.x;
        e = dir&1? e - scene_size/20: e + scene_size/20;

        if (shift && selObj<car.size())
            car[selObj]->move(t);
        else if (!shift && selObj<armadillo.size())
            armadillo[selObj]->move(t);
    }
}

void GlVisuals::mousePressed(int x, int y, int modif)
{
    mouselastX = x;
    mouselastY = y;
}

void GlVisuals::mouseMoved(int x, int y, int modif)
{
    int dx = x - mouselastX;
    int dy = y - mouselastY;

    //if (selObj<0){
    if (!modif) {
        globalRot.y += (dx>>1);
        globalRot.z += (dy>>1);
    } else {
        globalRot.y += (dx>>1);
        globalRot.x += (dy>>1);
    }

    mouselastX = x;
    mouselastY = y;
}

void GlVisuals::mouseWheel(int dir, int modif)
{
    bool ctrl  =  modif & 0x01;
    bool shift =  modif & 0x02;

    float &e = globalTranslation.z;
    e += scene_size/20 * (dir?-1:+1);
}
