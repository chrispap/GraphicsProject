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

GlVisuals::GlVisuals():
    globalTranslation (0,0,0),
    globalRot (0,0,0),
    perspective_proj (1),
    scene_size (100),
    scene_dist (scene_size),
	selObj (-1),
	selT ('z'),
    milli0 (-1),
    t (0.0)
{
	loadModels();
}

GlVisuals::~GlVisuals()
{
	for (int i=0; i<5; ++i) {
		delete armadillo[i];
		delete car[i];
		delete intersection[i];
	}
}

//[0] Manage models
void GlVisuals::loadModels()
{
	/* Load Model 1 */
	cout << " * Armadillo * " << endl;
	armadillo[0] = new Mesh("Model_1.obj", 0, 0);
	armadillo[0]->alignLocalCenter();
	armadillo[0]->setSize(scene_size/2);
	for (int i=1; i<5; ++i) {
		armadillo[i] = new Mesh(*armadillo[i-1]);
		armadillo[i]->reduce();
	}
	cout << endl;

	/* Load Model 2 */
	cout << " * Car * " << endl;
	car[0] = new Mesh("Model_2.obj", 1, 0);
	car[0]->alignLocalCenter();
	car[0]->setSize(scene_size/3);
	for (int i=1; i<5; ++i) {
		car[i] = new Mesh(*car[i-1]);
		car[i]->reduce();
	}
	cout << endl;

	/* Create empty meshes */
	for (int i=0; i<5; ++i) 
		intersection[i] = new Mesh("");

	/* Translate meshes */
	float zt = armadillo[0]->getBox().getZSize();
	float xt = armadillo[0]->getBox().getXSize()/2;
	for (int i=0; i<5; ++i) {
		armadillo[i] -> translate(Point( xt, 0, -zt*(2-i)));
		car[i] -> translate(Point( -xt, 0, -zt*(2-i)));
	}

}

void GlVisuals::intersectModels()
{
	cout << " * Intersections * " << endl;

	float t = armadillo[0]->getBox().getZSize();

	if (selObj>=0) {
		for (int i=0; i<5; ++i) {
			delete intersection[i];
			intersection[i] = new Mesh(*armadillo[selObj], *car[i], 1);
			//intersection[i]-> setLocalTranslation(Point( 0,0, -t*(2-i)));
		}
	}
}

//[1] OpenGL Callback Methods 
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
    //glEnable(GL_LINE_SMOOTH);
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

void GlVisuals::glPaint()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* Apply global transformations */
	glTranslatef(globalTranslation.x, globalTranslation.y, globalTranslation.z-scene_dist);
	glRotatef(globalRot.x, 1, 0, 0);
	glRotatef(globalRot.y, 0, 1, 0);
	glRotatef(globalRot.z, 0, 0, 1);
	drawAxes();
	drawScene();
}

//[2] Drawing Methods 
void GlVisuals::drawAxes()
{
	glBegin(GL_LINES);
	// [X]
	glColor3ub(0xFF, 0, 0);
	glVertex2f(0.0,0.0);
	glVertex2f(10.0*scene_size,0.0);
	// [Y]
	glColor3f(0, 0xFF, 0);
	glVertex2f(0.0,0.0);
	glVertex2f(0.0,10.0*scene_size);
	// [Z]
	glColor3f(0, 0, 0xFF);
	glVertex2f(0.0,0.0);
	glVertex3f(0.0,0.0,10.0*scene_size);

	glEnd();
}

void GlVisuals::drawScene()
{
	GLubyte c = 0x66;
	Colour col(c,c,c);
	for (int i=0; i<5; ++i) {
		armadillo[i]->draw(Colour(c,c,0), SOLID | (i==selObj?AABB:0));
		car[i]->draw(Colour(0,c,c), SOLID );
		intersection[i]->draw(Colour(c,0,c), SOLID | WIRE);
	}
}

//[3] UI Methods
static void qNormalizeAngle(int &angle)
{
	while (angle < 0)
		angle += 360;
	while (angle > 360)
		angle -= 360;
}

void GlVisuals::setEllapsedMillis(int millis)
{
	if (milli0<0)	// In the first call calibrate.
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
		else if (key>='1' && key <= '5') selObj = key-'0'-1;
		else if (key=='i') intersectModels();
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
		float &e = selT=='x'? globalTranslation.x : (selT=='y'? globalTranslation.y : globalTranslation.z);
		e += scene_size/20 * (dir?-1:+1);
	}
	else 
	{
		Point t(0,0,0);
		float &e = selT=='x'? t.x : (selT=='y'? t.y: t.z);
		e += scene_size/20 * (dir?-1:+1);
		armadillo[selObj]->translate(t);
	}

}
