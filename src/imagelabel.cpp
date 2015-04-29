#include "imagelabel.h"


    void ImageLabel::update()
    {
        if (!full_pixmap.isNull())
        {
        if (autoResizeWidget)
        {
                    this->setPixmap(full_pixmap.scaled(zoomFactor*full_pixmap.width(),
                        zoomFactor*full_pixmap.height(),Qt::KeepAspectRatio));
            this->resize(zoomFactor*full_pixmap.width(), zoomFactor*full_pixmap.height());
        }
                else
                {
                    this->setPixmap(full_pixmap.scaled(zoomFactor*width(), zoomFactor*height(),Qt::KeepAspectRatio));
                }
        }
//        std::cerr<<"QLabel "<<width()<<"x"<<height()<<" - QPixmap "<<pixmap()->width()<<"x"<<pixmap()->height()<<std::endl;
    }
void ImageLabel::changeColorClick(int c)
{
   
    color=c;
    setColorBlush(QColor::fromRgb(0, 255-color,255));
    
}

    void ImageLabel::resizeEvent(QResizeEvent* event)
    {
        if(!autoResizeWidget) update();
        QWidget::resizeEvent(event);
    }

    void ImageLabel::mouseMoveEvent(QMouseEvent *event)
    {
       //  printf("PAINT: %d ImageLabel MOVE: event: %d\n",paint,event->button());
        
        if (!full_pixmap.isNull())
        {
             _x = event->x()/zoomFactor;
             _y = event->y()/zoomFactor;
            //TODO: wrong values when not autoResizing
            emit mousePixelChanged(_x, _y);
        }
     //   QLabel::mouseMoveEvent(event);
        
      //  printf("MoveMouse: event: %d\n",event->button());
        
        
        if (paint) drawRec(event);
        
        

    }

    void ImageLabel::mousePressEvent(QMouseEvent *event)
    {
       // _x = event->x();
       // _y = event->y();
    //   printf("ImageLabel PRESS: event: %d\n",event->button());
        
        if (!full_pixmap.isNull())
        {
             _x = event->x()/zoomFactor;
             _y = event->y()/zoomFactor;

            emit mousePixelDown(_x, _y,event);

        }
        
        if (event->button()== Qt::LeftButton)
        {
            paint=true;
            drawRec(event);
        }
        
        QLabel::mousePressEvent(event);
        
    }

    void ImageLabel::mouseReleaseEvent(QMouseEvent *event)
    {
        if (!full_pixmap.isNull())
        {
            _x = event->x()/zoomFactor;
            _y = event->y()/zoomFactor;

            emit mousePixelUp(_x,_y,event);

        }
        
        paint=false;
        
        QLabel::mousePressEvent(event);
    }

void ImageLabel::drawRec(QMouseEvent *event)
{
    QPainter painter(&full_pixmap);
    painter.setBrush(QBrush(colorBlush));
    painter.drawEllipse(_x, _y,10,10);
    update();
}

    void ImageLabel::setColorBlush(QColor color)
    {
        //cambiar color del pixel
        colorBlush = color;
    }

    void ImageLabel::setZoom(double factor)
    {
        zoomFactor = factor;
        update();
    }

    void ImageLabel::zoom(double factor)
    {
        setZoom(zoomFactor*factor);
    }

    double ImageLabel::getZoom() const
    {
        return zoomFactor;
    }

    void ImageLabel::setOriginal(QImage& qimage)
    {
        original = qimage;
    }

    void ImageLabel::resetOriginal()
    {
        setImage(original);
    }


    void ImageLabel::setImage(const QImage& qimage)
    {
         full_pixmap = QPixmap::fromImage(qimage);
         setZoom();
         if (autoResizeWidget) this->setPixmap(full_pixmap);
    }

    void ImageLabel::setAutoResizeWidget(bool b)
    {
        autoResizeWidget = b;
    }
