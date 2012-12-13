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
#include <GL/gl.h>
#include <GL/glu.h>
#endif

using namespace std;

const float GlVisuals::PI=3.14159;

GlVisuals::GlVisuals()
{
/*Init params */
	milli0 = -1;
	t = 0.0;
    perspective_proj = 1;
    scene_size = 100;

/* Load Mesh 1 */
	armadillo[0] = new Mesh("Model_1.obj", 1);
	armadillo[0]->alingCenterToOrigin();
	armadillo[0]->setSize(scene_size/2);
	
	for (int i=1; i<5; ++i) {
		armadillo[i] = new Mesh(*armadillo[i-1]);
		armadillo[i]->reduce();
	}

	float t = armadillo[0]->getBox().getZSize();

	for (int i=0; i<5; ++i) {
		armadillo[i]->translate(Point( 0,0, t*(2-i)));
	}

	cout << endl;

/* Load Mesh 2 */
	car[0] = new Mesh("Model_2.obj", 1);
	car[0]->alingCenterToOrigin();
	car[0]->setSize(scene_size/3);
		
	for (int i=1; i<5; ++i) {
		car[i] = new Mesh(*car[i-1]);
		car[i]->reduce();
	}

	for (int i=0; i<5; ++i) {
		car[i]->translate(Point( 0,0, t*(2-i)));
	}

	cout << endl;

/* Create new model from the intersections of models 1,2 */
	for (int i=0; i<5; ++i) {
		intersection[i] = new Mesh(*armadillo[i], *car[i], 1);
	}
	
/* Set viewing angle | zoom */
    setXRotation(0);
    setYRotation(180);
    setZRotation(0);
    setDistancePercent(45);
    setHeightPercent(50);
}

GlVisuals::~GlVisuals()
{
	// delete models
}

/* OpenGL Callback Methods */
void GlVisuals::glPaint()
{
	glClear(GL_COLOR_BUFFER_BIT );
	glClear(GL_DEPTH_BUFFER_BIT );
	glMatrixMode(GL_MODELVIEW);
	
	/*static const int th0 = yRot;
	setYRotation(th0+t*30);*/
	
	/* Apply camera transformations */
	glLoadIdentity();
	glTranslatef(0.0, scene_height,	scene_dist);
	glRotated(xRot, 1.0, 0.0, 0.0);
	glRotated(yRot, 0.0, 1.0, 0.0);
	glRotated(zRot, 0.0, 0.0, 1.0);
	
	/* Draw scene objects */
	GLubyte c = 0x66;
	drawAxes();
	
	for (int i=0; i<5; ++i) {
		glColor3ub(c,c,0);
		armadillo[i]->draw(  SOLID | WIRE | AABB);

		glColor3ub(0,c,c);
		car[i]->draw( SOLID | WIRE | AABB);

		glColor3ub(c,0,c);
		intersection[i]->draw( SOLID );
	}

}

void GlVisuals::glInitialize()
{
    GLfloat ambientLight[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat diffuseLight[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat lightPos[] = { 0.0, 0.0, -scene_size, 0.0 };

    glLightfv( GL_LIGHT0, GL_AMBIENT, ambientLight );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuseLight );
    glLightfv( GL_LIGHT0, GL_POSITION, lightPos );

    glShadeModel( GL_FLAT );
    glEnable(GL_DEPTH_TEST);
    glDepthFunc( GL_LEQUAL );
    glClearDepth(1.0);
	
	//glEnable (GL_BLEND);
	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_CULL_FACE);
	//glEnable(GL_LINE_SMOOTH);
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
        if (h>w) glOrtho(-scene_size, scene_size, -scene_size*h/w, scene_size*h/w, -10.0f*scene_size, 10.0f*scene_size);
		else glOrtho(-scene_size*w/h, scene_size*w/h, -scene_size, scene_size, -10.0f*scene_size, 10.0f*scene_size);
    } else {
        if (h==0) h=1;
        glViewport(0,0,w,h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = (float)w/(float)h;	// aspect ratio
        gluPerspective(60.0, aspect, 1.0, 100.0*scene_size);
    }
}
/* Drawing Methods */
void GlVisuals::drawAxes()
{
    glBegin(GL_LINES);
    // [X]
    glColor3f(1.0, 0.0, 0.0);
    glVertex2f(0.0,0.0);
    glVertex2f(10.0*scene_size,0.0);
    // [Y]
    glColor3f(0.0, 1.0, 0.0);
    glVertex2f(0.0,0.0);
    glVertex2f(0.0,10.0*scene_size);
    // [Z]
    glColor3f(0.0, 0.0, 1.0);
    glVertex2f(0.0,0.0);
    glVertex3f(0.0,0.0,10.0*scene_size);

    glEnd();
}

/* UI Methods */
static void qNormalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360;
    while (angle > 360)
        angle -= 360;
}

void GlVisuals::setXRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != xRot)
        xRot = angle;
}

void GlVisuals::setYRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != yRot)
        yRot = angle;
}

void GlVisuals::setZRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != zRot)
        zRot = angle;
}

void GlVisuals::setDistancePercent(int d)
{
    scene_dist = (((float)d-50)/100)*10*scene_size;
}

void GlVisuals::setHeightPercent(int h)
{
    scene_height = (((float)h-50)/100)*1*scene_size;
}

void GlVisuals::setEllapsedMillis(int millis)
{
    if (milli0<0)	// In the first call of this method, calibrate.
        milli0 = millis;
    else
        t = ((millis-milli0)/1000.0);
}

void GlVisuals::mousePressed(int x, int y)
{
    mouse_lastX = x;
    mouse_lastY = y;
}

void GlVisuals::mouseMoved(int x, int y, int dir)
{
    int dx = x - mouse_lastX;
    int dy = y - mouse_lastY;

    if (!dir) {
      setYRotation(yRot + 1 * (dx>>1));
      setZRotation(zRot + 1 * (dy>>1));
    } else {
      setYRotation(yRot + 1 * (dx>>1));
      setXRotation(xRot + 1 * (dy>>1));
    }

    mouse_lastX = x;
    mouse_lastY = y;
}

void GlVisuals::mouseWheel(int dir)
{
    scene_dist = dir? scene_dist*1.1: scene_dist/1.1;
}
