#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include "glvisuals.h"

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

    QSize minimumSizeHint() const { return QSize(50, 50);}
    QSize sizeHint() const { return QSize(800, 600);}

public slots:
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);

signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);

protected:
    void initializeGL(){ visuals.glInitialize();}
    void paintGL(){ visuals.glPaint();}
    void resizeGL(int w, int h){ visuals.glResize(w,h);}
    void mousePressEvent(QMouseEvent *event){ visuals.mousePressed(event->x(), event->y());updateGL();}
    void mouseMoveEvent(QMouseEvent *event) {visuals.mouseMoved(event->x(), event->y());updateGL();}
    void wheelEvent(QWheelEvent *event) { visuals.mouseWheel(event->delta()>0?1:0);updateGL();}
    void keyPressEvent(QKeyEvent *event) { keyEvent(event, false);}
    void keyReleaseEvent(QKeyEvent *event) { keyEvent(event, true);}
    void keyEvent(QKeyEvent *event, bool up=false);

private:
    GlVisuals visuals;
};

#endif
