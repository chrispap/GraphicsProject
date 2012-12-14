#include <stdio.h>
#include "glvisuals.h"

#ifdef __linux__
#include <GL/glut.h>
#else
#include "gl/glut.h"
#endif

GlVisuals *visuals;

void timerFired(int val)
{
	glutPostRedisplay();
}

void Render()
{
	//glutTimerFunc(5, timerFired, 0);
	visuals->setEllapsedMillis(glutGet(GLUT_ELAPSED_TIME));
	visuals->glPaint();
	glutSwapBuffers();
}

void Resize(int w, int h)
{
	visuals->glResize(w, h);
}

int Modif()
{
	int modif = glutGetModifiers();
	unsigned char ctrl =  (modif & GLUT_ACTIVE_CTRL)?  1: 0;
	unsigned char shift = (modif & GLUT_ACTIVE_SHIFT)? 1: 0;
	return (shift<<1) | (ctrl<<0);
}

void mouseEvent(int button, int state, int x, int y)
{
	if (state == GLUT_UP) 
		return;	
	
	if ((button == 3) || (button == 4)) { // Wheel event
		visuals->mouseWheel(button==3, Modif());
		glutPostRedisplay();
	
	} else { // Click event
		visuals->mousePressed(x,y);
	}
}

void mouseMotion(int x, int y)
{
	int modif = glutGetModifiers();
	visuals->mouseMoved(x,y, Modif());
	glutPostRedisplay();
}

void KeyEvent(unsigned char key, int x, int y, bool updown)
{
	visuals->keyEvent(key, x, y, updown);
	glutPostRedisplay();
}

void KeyUpEvent(unsigned char key, int x, int y)
{
	KeyEvent(key, x, y, true);
}

void KeyDownEvent(unsigned char key, int x, int y)
{
	KeyEvent(key, x, y, false);
}

void SpeciaEvent (int key, int x, int y)
{
	if      (key == GLUT_KEY_UP) visuals->arrowEvent(0, Modif());
	else if (key == GLUT_KEY_DOWN) visuals->arrowEvent(1, Modif());
	else if (key == GLUT_KEY_RIGHT) visuals->arrowEvent(2, Modif());
	else if (key == GLUT_KEY_LEFT) visuals->arrowEvent(3, Modif());
	else return;
	
	glutPostRedisplay();
}

int main(int argc, char* argv[])
{
    visuals = new GlVisuals();
    
	/* Init GLUT */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(0,0);
	glutCreateWindow("Project 6609");
	
	/*Set GLUT callbacks */
	glutDisplayFunc(Render);
	glutReshapeFunc(Resize);
	glutMouseFunc(mouseEvent);
	glutMotionFunc(mouseMotion);
	glutKeyboardFunc(KeyDownEvent);
	glutKeyboardUpFunc(KeyUpEvent);
	glutSpecialFunc(SpeciaEvent);
	
	
	/* Init our "scene's" OpenGL Parameters */
    visuals->glInitialize();
	
	/* Enter main loop */
	//glutFullScreen();
	glutMainLoop();
	
	return 0;
}
