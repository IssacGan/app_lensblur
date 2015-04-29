#include <QApplication>

#include "imageviewer2.h"
#include <string>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"

 int main(int argc, char *argv[])
 {
     QApplication app(argc, argv);
     
     ImageViewer imageViewer;
     imageViewer.show();

     return app.exec();
 }
