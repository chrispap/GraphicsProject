#ifndef WINDOW_H
#define WINDOW_H

#include <QMainWindow>
#include <QSlider>
#include "glwidget.h"

class Window : public QMainWindow {
    Q_OBJECT

public:
    Window();
    ~Window();

private:
    QWidget *centralWidget;
    GLWidget *glWidget;
    QSlider *xSlider;
    QSlider *ySlider;
    QSlider *zSlider;
    QSlider *createSlider(int range);
};

#endif // WINDOW_H
