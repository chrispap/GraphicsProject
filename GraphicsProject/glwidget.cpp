#include <QtGui>
#include <QtOpenGL>
#include "glwidget.h"

GLWidget::GLWidget(QWidget *parent):
    QGLWidget(QGLFormat(QGL::DoubleBuffer), parent)
{
    setFocusPolicy(Qt::StrongFocus);

}

GLWidget::~GLWidget()
{

}

void GLWidget::setXRotation(int angle)
{
    Point rot = visuals.getGlobalRotation();
    rot.x = (float)angle;
    visuals.setGlobalRotation(rot);
    updateGL();
}

void GLWidget::setYRotation(int angle)
{
    Point rot = visuals.getGlobalRotation();
    rot.y = (float)angle;
    visuals.setGlobalRotation(rot);
    updateGL();
}

void GLWidget::setZRotation(int angle)
{
    Point rot = visuals.getGlobalRotation();
    rot.z = (float)angle;
    visuals.setGlobalRotation(rot);
    updateGL();
}

void GLWidget::keyEvent(QKeyEvent *evt, bool up)
{
    if (evt->text().length()>0) {
        char c = evt->text().toAscii()[0];
        visuals.keyEvent(c, up);
    } else {
        switch (evt->key()){
        case Qt::Key_Up:    visuals.arrowEvent(UP); break;
        case Qt::Key_Down:  visuals.arrowEvent(DOWN); break;
        case Qt::Key_Right: visuals.arrowEvent(RIGHT); break;
        case Qt::Key_Left:  visuals.arrowEvent(LEFT); break;
        case Qt::Key_X:     visuals.keyEvent('x', up); break;
        case Qt::Key_Y:     visuals.keyEvent('y', up); break;
        case Qt::Key_Z:     visuals.keyEvent('z', up); break;
        default: return;
        }
    }

    updateGL();
}
