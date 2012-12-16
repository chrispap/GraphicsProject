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

const float GlVisuals::PI=3.14159;
const int GlVisuals::N=5;

GlVisuals::GlVisuals():
    armadillo(N),
    car(N),
    intersection(N),
    globalTranslation (0,0,0),
    globalRot (45,0,0),
    perspective_proj (1),
    scene_size (100),
    scene_dist (scene_size),
    selObj (-1),
    selT ('z'),
    milli0 (-1),
    t (0.0)
{
    loadScene();
}

GlVisuals::~GlVisuals()
{
    for (int i=0; i<N; ++i) {
        delete armadillo[i];
        delete car[i];
        delete intersection[i];
    }
}

/** Manage scene */
void GlVisuals::loadScene()
{
    /* Load Model 1 */
    cout << " * Armadillo * " << endl;
    armadillo[0] = new Mesh("Model_1.obj", 0, 0);
    armadillo[0]->setSize(scene_size/2);
    printf("Mesh volume coverage:\t%4.2f%% \n", 100*armadillo[0]->getBoundingCoverage());

    for (int i=1; i<N; ++i) {
        armadillo[i] = new Mesh(*armadillo[i-1]);
        armadillo[i]->reduce();
    }

    cout << endl;

    /* Load Model 2 */
    cout << " * Car * " << endl;
    car[0] = new Mesh("Model_2.obj", 1, 0);
    car[0]->setSize(scene_size/3);
    printf("Mesh volume coverage:\t%4.2f%% \n", 100*car[0]->getBoundingCoverage());

    for (int i=1; i<N; ++i) {
        car[i] = new Mesh(*car[i-1]);
        car[i]->reduce();
    }
    cout << endl;

    /* Translate meshes */
    float zt = armadillo[0]->getBox().getZSize();
    float xt = armadillo[0]->getBox().getXSize()/2;
    for (int i=0; i<N; ++i) {
        armadillo[i] -> translate(Point( xt, 0, -zt*((N/2)-i)));
        car[i] -> translate(Point( -xt, 0, -zt*((N/2)-i)));
    }

    /* Create empty meshes */
    for (int i=0; i<N; ++i)
        intersection[i] = new Mesh("");
}

void GlVisuals::drawScene()
{
    //armadillo[0]->draw (Colour(0x66,0x66,0), SOLID | VOXELS | (0==selObj?AABB:0));return;
    for (int i=0; i<N; ++i) {
        car[i]         ->draw (Colour(0,0x66,0x66), SOLID | VOXELS | (i==selObj?AABB:0));
        armadillo[i]   ->draw (Colour(0x66,0x66,0), SOLID | VOXELS | (i==selObj?AABB:0));
        intersection[i]->draw (Colour(0x66,0,0x66), SOLID | WIRE  | (i==selObj?AABB:0));
    }
}

void GlVisuals::intersectScene()
{
    cout << " * Intersections * " << endl;

    float t = armadillo[0]->getBox().getZSize();

    if (selObj>=0) {
        for (int i=0; i<N; ++i) {
            delete intersection[i];
            intersection[i] = new Mesh(*armadillo[selObj], *car[i], 1);
        }
    }
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
    drawAxes();
    /* Apply global transformations */
    glTranslatef(globalTranslation.x, globalTranslation.y, globalTranslation.z);
    glRotatef(globalRot.x, 1, 0, 0);
    glRotatef(globalRot.y, 0, 1, 0);
    glRotatef(globalRot.z, 0, 0, 1);
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

void GlVisuals::keyEvent (unsigned char key,  bool up, int x, int y)
{
    key = tolower(key);

    if (up) selObj = -1;
    else {
        if (key>='x' && key <='z') selT = key;
        else if (key>='1' && key <= ('0'+N)) selObj = key-'0'-1;
        else if (key=='i') intersectScene();
    }
}

void GlVisuals::arrowEvent (int dir, int modif)
{
    if (selObj<0)
    {
        Point &t = globalTranslation;
        float &e = dir&2? t.x : t.y;
        e = dir&1? e - scene_size/20: e + scene_size/20;
    }
    else
    {
        Point t(0,0,0);
        float &e = dir&2? t.z : t.x;
        e = dir&1? e - scene_size/20: e + scene_size/20;
        armadillo[selObj]->translate(t);
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
    if (selObj < 0)
    {
        float &e = globalTranslation.z;
        e += scene_size/20 * (dir?-1:+1);
    }
    else
    {
        Point t(0,0,0);
        float &e = t.y;
        e += scene_size/20 * (dir?-1:+1);
        armadillo[selObj]->translate(t);
    }

}
