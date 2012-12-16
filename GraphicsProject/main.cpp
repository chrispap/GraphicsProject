#include <stdio.h>
#include "glvisuals.h"

#ifdef __linux__
#include <GL/glut.h>
#else
#include "gl/glut.h"
#endif

static GlVisuals *visuals;
static int modif=0;

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

void mkModif(int *modif)
{
    unsigned char ctrl =  (*modif & GLUT_ACTIVE_CTRL)?  1: 0;
    unsigned char shift = (*modif & GLUT_ACTIVE_SHIFT)? 1: 0;
    *modif = (shift<<1) | (ctrl<<0);
}

void mouseEvent(int button, int state, int x, int y)
{
	modif = glutGetModifiers();
	mkModif(&modif);

    if (state == GLUT_UP) 
        return;    

    if ((button == 3) || (button == 4)) { // Wheel event
		cout << "???";
        visuals->mouseWheel(button==3, modif);
        glutPostRedisplay();

    } else { // Click event
        visuals->mousePressed(x,y,modif);
    }
}

void mouseMotion(int x, int y)
{
#ifdef __linux__
	modif = glutGetModifiers();
#endif
    mkModif(&modif);
    
    visuals->mouseMoved(x,y, modif);
    glutPostRedisplay();
}

void KeyEvent(unsigned char key, bool updown, int x, int y)
{
	modif = glutGetModifiers();
	mkModif(&modif);

    if (key==27 ) exit(0);
    visuals->keyEvent(key, updown, x, y, modif);
    glutPostRedisplay();
}

void KeyUpEvent(unsigned char key, int x, int y)
{
    KeyEvent(key, true, x, y);
}

void KeyDownEvent(unsigned char key, int x, int y)
{
    KeyEvent(key, false, x, y);
}

void SpeciaEvent (int key, int x, int y)
{
    ArrowDir dir;
	modif = glutGetModifiers();
	mkModif(&modif);

    if      (key == GLUT_KEY_UP) dir = UP;
    else if (key == GLUT_KEY_DOWN) dir = DOWN;
    else if (key == GLUT_KEY_RIGHT) dir = RIGHT;
    else if (key == GLUT_KEY_LEFT) dir = LEFT;
    else return;

    visuals->arrowEvent(dir, modif);
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
    glutMainLoop();

    return 0;
}
