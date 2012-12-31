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
    globTrans (globTrans0),
    globRot (globRot0),
    perspective_proj (1),
    scene_size (100),
    scene_dist (scene_size*0.5),
    sel_i (-1),
    sel_obj (0),
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
    puts("\n ==== Armadillo ====");
    armadillo.push_back (new Mesh("Model_1.obj", 1));
    armadillo[0]->setMaxSize(scene_size/2);

    puts("\n ==== Car ====");
    car.push_back (new Mesh("Model_2.obj"));
    car[0]->setMaxSize(scene_size/3);

    //intersectScene();
}

void GlVisuals::resetScene()
{
    Point zero(0,0,0);

    for (int i=1; i<armadillo.size(); ++i) delete armadillo[i];
    armadillo.resize(1);
    armadillo[0]->setPos(zero);

    for (int i=1; i<car.size(); ++i) delete car[i];
    car.resize(1);
    car[0]->setPos(zero);

    for (int i=0; i<intersection.size(); ++i) delete intersection[i];
    intersection.resize(0);

    globRot = globRot0;
    globTrans = globTrans0;

    intersectScene();
}

void GlVisuals::intersectScene()
{
    for (int i=0; i<intersection.size(); ++i)
        delete intersection[i];

    intersection.clear();

    for (int i=0; i<armadillo.size(); ++i) {
        for (int j=0; j<car.size(); ++j)
            intersection.push_back (new Mesh(*armadillo[i], *car[j], 1));

        for (int k=i+1; k<armadillo.size(); ++k)
            intersection.push_back (new Mesh(*armadillo[i], *armadillo[k], 1));
    }
}

void GlVisuals::duplicateObject(int obj)
{
    vector<Mesh*> &model = obj==0? armadillo: car;

    //~ model.push_back (new Mesh(*model.back()));
    //~ Point mov = Point(model.back()->getBox().getSize());
    //~ mov.x=0;mov.y=0;
    //~ model.back()->move(mov);
    //~ sel_i = model.size()-1;
    model.back()->simplify(1);
    intersectScene();
}

void GlVisuals::drawScene()
{
    for (int i=0; i<armadillo.size(); ++i)
        armadillo[i]->draw (Colour(0x66,0x66,0), style | ((i==sel_i)?AABB:0));

    for (int i=0; i<car.size(); ++i)
        car[i]->draw (Colour(0,0x66,0x66), style | ((i==sel_i)?AABB:0));

    for (int i=0; i<intersection.size(); ++i)
        intersection[i]->draw (Colour(0x66,0,0x66), SOLID | WIRE);
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


/** OpenGL Callback Methods */
void GlVisuals::glInitialize()
{
    static GLfloat ambientLight[] = { 1, 1, 1, 1 };
    static GLfloat diffuseLight[] = { 1, 1, 1, 1 };
    static GLfloat lightPos[] =     { 1, 1, 1, 0 };

    glLightfv( GL_LIGHT0, GL_AMBIENT, ambientLight );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuseLight );
    glLightfv( GL_LIGHT0, GL_POSITION, lightPos );

    glShadeModel( GL_SMOOTH );
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
    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
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
    glTranslatef(globTrans.x, globTrans.y, globTrans.z);
    glRotatef(globRot.x, 1, 0, 0);
    glRotatef(globRot.y, 0, 1, 0);
    glRotatef(globRot.z, 0, 0, 1);
    drawAxes();
    drawScene();
}

void GlVisuals::enterPixelMode()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screen_width, screen_height, 0, 10, -10);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
}

void GlVisuals::returnFromPixelMode()
{
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
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
    bool ctrl  =  modif & 0x01;
    bool shift =  modif & 0x02;

    key = tolower(key);

    if (up) {
        //sel_i = -1;
    }
    else {
        if (isdigit(key)) { sel_i = key-'0'-1; sel_obj = (shift?1:0);}
        else if (key=='0') sel_i = -1;
        else if (key=='d') duplicateObject(shift?1:0);
        else if (key=='r') resetScene();
        else if (key=='i') intersectScene();
        else if (key=='s') style ^= SOLID;
        else if (key=='w') style ^= WIRE;
        else if (key=='n') style ^= NORMALS;
        else if (key=='p') style ^= SPHERE;
        else if (key=='b') style ^= AABB;
        else if (key=='t') style ^= TBOXES;
        else if (key=='v') style ^= VOXELS;
        else if (key=='h') style ^= HIER;
    }

}

void GlVisuals::arrowEvent (int dir, int modif)
{
    bool ctrl  =  modif & 0x01;
    bool shift =  modif & 0x02;

    if (sel_i<0)
    {
        Point &t = globTrans;
        float &e = ctrl? t.z : dir&2? t.x : t.y;
        e = dir&1? e - scene_size/50: e + scene_size/50;
    }
    else
    {
        Point t(0,0,0);
        float &e = ctrl? t.y : dir&2? t.z : t.x;
        e = dir&1? e - scene_size/50: e + scene_size/50;

        if (shift && sel_i<car.size())
            car[sel_i]->move(t);
        else if (!shift && sel_i<armadillo.size())
            armadillo[sel_i]->move(t);

        //it slows things down but never mind...
        intersectScene();
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

    //if (sel_i<0){
    if (!modif) {
        globRot.y += (dx>>1);
        globRot.z += (dy>>1);
    } else {
        globRot.y += (dx>>1);
        globRot.x += (dy>>1);
    }

    mouselastX = x;
    mouselastY = y;
}

void GlVisuals::mouseWheel(int dir, int modif)
{
    bool ctrl  =  modif & 0x01;
    bool shift =  modif & 0x02;

    float &e = globTrans.z;
    e += scene_size/20 * (dir?-1:+1);
}
