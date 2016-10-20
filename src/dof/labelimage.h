#ifndef LABEL_IMAGE
#define LABEL_IMAGE

//#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QMouseEvent>
#include <QImage>

class LabelImage : public QLabel
{
    Q_OBJECT
private:
    QString name;
    QImage image;//image original sin cambiar su tamaño
    QPixmap pixmap;

    //draw
    bool canEdit = false;
    bool paint = false;
    QColor colorBlush = QColor::fromRgb(0, 0,0);
    QPainter painter;

    //pixel selected
    int _x=0;
    int _y=0;
    
    //resize
   // double zoomFactor=1.0;
    
    

public:
    //explicit imageLabel(QWidget *parent = 0);
    LabelImage()
    {
        this->setBackgroundRole(QPalette::Base);
       // this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        this->setAlignment(Qt::AlignCenter);
        this->setMouseTracking(true);
        //this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        this->setScaledContents(true);
        
        //this->setAlignment(Qt::AlignCenter);
       // this->setMouseTracking(true);
    }

    //init imagen
    bool initImageLabel(QString nameIm="", int w=640, int h=320 )
     {
         name = nameIm;
         image.load(nameIm);
         
         
         //ajustar al tamño original
        // QSize tamWidget = parentWidget()->size();
         
        // float newHeight = (float)image.height()/(float)image.width() * (float)parentWidget()->size().width();
         
         pixmap = QPixmap::fromImage(image);
         //this->setPixmap(QPixmap::fromImage(image));
         this->setPixmap(pixmap);//.scaled(image.width(), image.height(),Qt::KeepAspectRatio));
         resize(sizeHint());//this->adjustSize();
         
         
       //  QSize tamWidget= parentWidget()->size();
       // image = image.scaled(tamWidget.width(),tamWidget.height(),Qt::KeepAspectRatio);
       //  this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        // setMinimumSize(image.width(),image.height());//image.width(),image.height());

         
        // this->setPixmap(QPixmap::fromImage(image));

         if (image.isNull()) return false;
         else return true;
    }
    
    void setImage(QImage image)
    {
        setPixmap(QPixmap::fromImage(image));
    }
    
    //activar solo desde fuera
    void setCanEdit(bool status)
    {
        canEdit = status;
    }
    
    bool getCanEdit()
    {
        return canEdit;
    }
    
    void enablePaint()
    {
        paint=true;
    }
    
    void disablePaint()
    {
        paint=false;
    }

    //DRAW over image
    void setColorBlush(QColor color)
    {
        //cambiar color del pixel del raton
        colorBlush = color;
    }

    //resetear la imagen original. Borrar las ediciones
    void resetImage()
    {
        pixmap = QPixmap::fromImage(image);
        this->setPixmap(QPixmap::fromImage(image));
    }

//protected:

    //Eventos MOUSE
    void mouseMoveEvent(QMouseEvent *event)
    {
        QSize tamOri = image.size();
        QSize tamWidget = parentWidget()->size();
        float zoomFactor = (float)tamWidget.width() / (float)tamOri.width() ;
        
         if (!pixmap.isNull() and paint)
         {
             _x = (int) ((float) event->x() / zoomFactor);//event->x();///zoomFactor;
             _y = (int) ((float)event->y() / zoomFactor); //event->y();///zoomFactor;


         }
        
      //  fprintf(stderr,"EVENT x: %d y: %d\n ", event->x(), event->y());
      //  fprintf(stderr,"real x: %d y: %d\n ",(int) ((float) event->x() / zoomFactor), (int) ((float)event->y()/zoomFactor));

         if (paint)
         {
             drawStroke(event);
             
             emit mousePixelChanged(_x, _y);
             
         }
    }

    void mousePressEvent(QMouseEvent *event)
    {
       // fprintf(stderr,"Press x: %d y: %d\n ", _x, _y);
        QSize tamOri = image.size();
        QSize tamWidget = parentWidget()->size();
        float zoomFactor = (float)tamWidget.width() / (float)tamOri.width() ;

        if (!pixmap.isNull())
        {
            _x = (int) ((float) event->x() / zoomFactor);//event->x();///zoomFactor;
            _y = (int) ((float)event->y() /zoomFactor); //event->y();///zoomFactor;

            
             if (event->button()== Qt::LeftButton)
             {
                 enablePaint();
                 drawStroke(event);
                 emit mousePixelDown(_x, _y, event);
             }
             else if (event->button()== Qt::RightButton)
             {
               resetImage();
               disablePaint();
               //fprintf(stderr,"Antes de emitir la señal");
               emit removeImageStrokes();

             }
            
        }
        emit mousePixelChanged(_x, _y);

    }
    
    void mouseReleaseEvent(QMouseEvent *event)
    {
        //fprintf(stderr,"Unpress x: %d y: %d\n ", _x, _y);
        QSize tamOri = image.size();
        QSize tamWidget = parentWidget()->size();
        float zoomFactor = (float)tamWidget.width() / (float)tamOri.width() ;

         if (!pixmap.isNull())
         {
             _x = (int) ((float) event->x() / zoomFactor);//event->x();///zoomFactor;
             _y = (int) ((float)event->y()/zoomFactor); //event->y();///zoomFactor;

             emit mousePixelUp(_x, _y,event);

         }
        disablePaint();
        emit mousePixelChanged(_x, _y);

    }

    //dibujar
    void drawStroke(QMouseEvent *event)
    {
        if (paint & canEdit)
        {         
            QPainter painter(&pixmap);
            painter.setBrush(QBrush(colorBlush));
            
            QSize tamOri = image.size();
            painter.drawEllipse(_x, _y,0.025*tamOri.width(),0.025*tamOri.width());
            this->setPixmap(pixmap);
           }
    }
    
    void resizeEvent(QResizeEvent* event)
    {
        //calcular zoom ratio
        QSize tamOri = image.size();
        
        if (tamOri.width()!=0)
        
        {
            float newHeight = (float)tamOri.height()/(float)tamOri.width() * (float)parentWidget()->size().width();
           // float newWidht = (float)tamOri.width()/(float)tamOri.height() * (float)parentWidget()->size().height();
      
        
            this->setPixmap(pixmap.scaled(parentWidget()->size().width(), newHeight,Qt::KeepAspectRatio));
            // this->setPixmap(pixmap.scaled(newWidht, parentWidget()->size().height(),Qt::KeepAspectRatio));
        
            this->adjustSize();
        }
        //fprintf(stderr,"resize label %d %d\n",image.width(),image.height());
    }

//señales a emitir y capturar desde fuera
signals:
    void mousePixelChanged(int x, int y);
    void mousePixelDown(int x, int y,QMouseEvent *event);
    void mousePixelUp(int x, int y,QMouseEvent *event);
    void removeImageStrokes();
    
};

#endif // LABEL_IMAGE
