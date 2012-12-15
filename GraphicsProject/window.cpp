#include <QWidget>
#include <QHBoxLayout>
#include <QSlider>
#include "window.h"
#include "glwidget.h"

Window::Window(QWidget *parent)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    glWidget = new GLWidget;
    xSlider = createSlider(360 * 2);
    ySlider = createSlider(360 * 2);
    zSlider = createSlider(360 * 2);
    connect(xSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setXRotation(int)));
    connect(ySlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setYRotation(int)));
    connect(zSlider, SIGNAL(valueChanged(int)), glWidget, SLOT(setZRotation(int)));
    connect(glWidget, SIGNAL(xRotationChanged(int)), xSlider, SLOT(setValue(int)));
    connect(glWidget, SIGNAL(yRotationChanged(int)), ySlider, SLOT(setValue(int)));
    connect(glWidget, SIGNAL(zRotationChanged(int)), zSlider, SLOT(setValue(int)));
    mainLayout->addWidget(glWidget);
    mainLayout->addWidget(xSlider);
    mainLayout->addWidget(ySlider);
    mainLayout->addWidget(zSlider);
    setLayout(mainLayout);
}

Window::~Window()
{

}

QSlider *Window::createSlider(int range)
{
    QSlider *slider = new QSlider(Qt::Vertical);
    slider->setRange(0, range);
    slider->setSingleStep(16);
    slider->setPageStep(15 * 2);
    slider->setTickInterval(15 * 2);
    slider->setTickPosition(QSlider::TicksRight);
    return slider;
}
