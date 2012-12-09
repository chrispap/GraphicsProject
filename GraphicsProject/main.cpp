#include "glvisuals.h"

#ifdef __linux__
#include <GL/glut.h>
#else
#include "gl/glut.h"
#endif

GlVisuals *visuals;

void timerFired(int val)
{
	glutTimerFunc(5, timerFired, 0);
	glutPostRedisplay();
}

void Render()
{
	visuals->setEllapsedMillis(glutGet(GLUT_ELAPSED_TIME));
	visuals->glPaint();
	glutSwapBuffers();
}

void Resize(int w, int h)
{
	visuals->glResize(w, h);
}

void mouseEvent(int button, int state, int x, int y)
{
	if (state == GLUT_UP) 
		return;	
	// Wheel event
	if ((button == 3) || (button == 4)) {
		visuals->mouseWheel(button==3);
		glutPostRedisplay();
	// Click event
	} else { 
		visuals->mousePressed(x,y);
	}
}

void mouseMotion(int x, int y)
{
	visuals->mouseMoved(x,y);
	glutPostRedisplay();
}

int main(int argc, char* argv[])
{
    visuals = new GlVisuals();
    
	/* Init GLUT */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
	glutInitWindowSize(1300, 700);
	glutInitWindowPosition(0,0);
	glutCreateWindow("Project 6609");
	
	/*Set GLUT callbacks */
	glutDisplayFunc(Render);
	glutReshapeFunc(Resize);
	glutMouseFunc(mouseEvent);
	glutMotionFunc(mouseMotion);
	
	/* Init our "scene's" OpenGL Parameters */
    visuals->glInitialize();
	
	/* Enter main loop */
	glutMainLoop();
	
	return 0;
}
