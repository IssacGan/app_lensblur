#include <QApplication>

#include <gui/windowfilter.h>
#include <filters/filterdepthoffield.h>

int main(int argc, char *argv[])
{
     QApplication app(argc, argv);
     
     FilterDepthOfField filter;
     WindowFilter window(filter);
     window.addBrush("Furthest",      1.0);
     window.addBrush("Far",          85.0);
     window.addBrush("Close",       170.0);
     window.addBrush("Near",        255.0);
     window.show();

     return app.exec();
}
