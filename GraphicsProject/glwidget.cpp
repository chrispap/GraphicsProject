#include <QtGui>
#include <QtOpenGL>
#include "glwidget.h"

 GLWidget::GLWidget(QWidget *parent)
     : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
 {

 }

 GLWidget::~GLWidget()
 {
 }

 QSize GLWidget::minimumSizeHint() const
 {
     return QSize(50, 50);
 }

 QSize GLWidget::sizeHint() const
 {
     return QSize(800, 600);
 }

 void GLWidget::setXRotation(int angle)
 {

 }

 void GLWidget::setYRotation(int angle)
 {

 }

 void GLWidget::setZRotation(int angle)
 {

 }

 void GLWidget::initializeGL()
 {

 }

 void GLWidget::paintGL()
 {

 }

 void GLWidget::resizeGL(int width, int height)
 {

 }

 void GLWidget::mousePressEvent(QMouseEvent *event)
 {

 }

 void GLWidget::mouseMoveEvent(QMouseEvent *event)
 {

 }
