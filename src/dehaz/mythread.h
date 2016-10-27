#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>

#include <QWidget>

#include "widgetImage.h"

class MyThread : public QThread
{
    Q_OBJECT

    WidgetImage *gui;
    
    QString path;
    QString dir;

    bool statusFin;
    int mode;

public:
    
    MyThread(WidgetImage *window);
    void setFile(QString file);
    void setDir(QString file);
    void setMode(int i);

protected:
    void run();
};

#endif // MYTHREAD_H
