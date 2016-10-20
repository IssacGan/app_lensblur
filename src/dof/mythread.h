#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>

#include <QWidget>

#include "widgetFocus.h"

class MyThread : public QThread
{
    Q_OBJECT

    WidgetFocus *gui;
    
    QString path;
    QString dir;

    bool statusFin;
    int mode;

public:
    
    MyThread(WidgetFocus *window);
    void setFile(QString file);
    void setDir(QString file);
    void setMode(int i);

protected:
    void run();
};

#endif // MYTHREAD_H
