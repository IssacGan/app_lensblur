#include <QApplication>

#include <gui/windowfilter.h>
#include <filters/filtertonemapping.h>

 int main(int argc, char *argv[])
 {
     QApplication app(argc, argv);
     
     FilterTonemapping filter;
     WindowFilter window(filter);
     window.addBrush("Darken",         255.0);
     window.addBrush("Preserve light", 127.0);
     window.addBrush("Light up",        15.0);
     window.show();

     return app.exec();
 }
