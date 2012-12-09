#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QSlider>
#include "glwidget.h"

class Window : public QMainWindow {
    Q_OBJECT

public:
    Window(QWidget *parent = 0);
    ~Window();

private:
    GLWidget *glWidget;
    QSlider *xSlider;
    QSlider *ySlider;
    QSlider *zSlider;
    QSlider *distSlider;
    QSlider *heightSlider;
    QSlider *createSlider(int range);
};

#endif // WINDOW_H
