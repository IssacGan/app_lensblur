#include <QApplication>

#include <gui/windowfilter.h>
#include <filters/filtertonemapping.h>
#include <filters/filtertonemappingcolor.h>

 int main(int argc, char *argv[])
 {
     QApplication app(argc, argv);
     
     FilterTonemappingColor filter;
     WindowFilter window(filter);
     window.show();

     return app.exec();
 }
