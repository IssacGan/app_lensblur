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

    bool statusFin;
    int mode;

public:
    
    MyThread(WidgetFocus *window);
    void getFile(QString file);
    void setMode(int i);

protected:
    void run();
};

#endif // MYTHREAD_H
