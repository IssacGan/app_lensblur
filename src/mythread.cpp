#include "mythread.h"

MyThread::MyThread(WidgetFocus *window)
{
    this->gui=window;
}

void  MyThread::getFile(QString file)
{
    path = file;
    statusFin=false;
}

void  MyThread::setMode(int i)
{
    mode = i;
}

void MyThread::run()
{
    switch (mode) {
        case 0:
            this->gui->loadData(path);
            break;
        case 1:
            this->gui->loadImage(path);
            break;
        case 2:
            this->gui->loadSuperPixels(path);
            break;
        case 3:
         //   this->gui->loadDepth(path);
            break;
        case 4:
            this->gui->loadDepth();
            break;
        case 5: //save
            this->gui->saveData();

        default:
            break;
    }
    
}
