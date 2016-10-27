#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QPainter>


class ImageLabel : public QLabel
{
    Q_OBJECT
private:

   
    double zoomFactor;
    bool autoResizeWidget;
    void update();
    void drawRec(QMouseEvent *event);
    
signals:
    void mousePixelChanged(int x, int y);
    void mousePixelDown(int x, int y,QMouseEvent *event);
    void mousePixelUp(int x, int y,QMouseEvent *event);
    
    
    
protected:
    void resizeEvent(QResizeEvent* event);

    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    //void paintEvent(QPaintEvent *event);
   

public:
    //todo
    int _x=0;
    int _y=0;
    //pasar a privado
    QPixmap full_pixmap;
    QImage original;
    
    bool paint = false;
    int color=255;
    
    QColor colorBlush;
    
    void setColorBlush(QColor color);
    
    QPainter painter;
    void changeColorClick(int c);
    
    void setZoom(double factor = 1.0);

    void zoom(double factor);

    double getZoom() const;

    void setImage(const QImage& qimage);
    
    void setOriginal(QImage& qimage);
    void resetOriginal();
    

    void setAutoResizeWidget(bool b);

    ImageLabel() : zoomFactor(1.0), autoResizeWidget(true)
    {
        this->setBackgroundRole(QPalette::Base);
      //  this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
       // this->setAlignment(Qt::AlignCenter);
        this->setMouseTracking(true);
        painter.begin(&full_pixmap);
    }

    ~ImageLabel() { }
};


#endif // IMAGELABEL_H
