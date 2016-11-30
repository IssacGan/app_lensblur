#include <QApplication>

#include <gui/windowfilter.h>
#include <filters/filterdepthoffield.h>

int main(int argc, char *argv[])
{
     QApplication app(argc, argv);
     
     FilterDepthOfField filter;
     WindowFilter window(filter);
     window.show();

     return app.exec();
}
