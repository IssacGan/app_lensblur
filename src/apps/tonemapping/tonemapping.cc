#include <QApplication>

#include <gui/windowfilter.h>
#include <filters/filtertonemapping.h>

 int main(int argc, char *argv[])
 {
     QApplication app(argc, argv);
     
     FilterTonemapping filter;
     WindowFilter window(filter);
     window.addBrush("Darken",          0.05);
     window.addBrush("Preserve light",  0.5);
     window.addBrush("Light up",        1.0);
     window.show();

     return app.exec();
 }
