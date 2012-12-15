#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QSlider>
#include "glwidget.h"

class Window : public QWidget {
    Q_OBJECT

public:
    Window(QWidget *parent = 0);
    ~Window();

private:
    GLWidget *glWidget;
    QSlider *xSlider;
    QSlider *ySlider;
    QSlider *zSlider;
    QSlider *createSlider(int range);
};

#endif // WINDOW_H
