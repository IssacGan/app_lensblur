#include <QApplication>

#include <gui/windowfilter.h>
#include <filters/filterdepthoffield.h>

int main(int argc, char *argv[])
{
     QApplication app(argc, argv);
     
     FilterDepthOfField filter;
     WindowFilter window(filter);
     window.addBrush("Furthest",     0.01);
     window.addBrush("Far",          0.34);
     window.addBrush("Close",        0.67);
     window.addBrush("Near",         1.0);
     window.show();

     return app.exec();
}
