#ifndef LABEL_IMAGE
#define LABEL_IMAGE

#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QMouseEvent>
#include <QImage>
#include <iostream>

class LabelImage : public QLabel
{
    Q_OBJECT
private:
    QString name;
    QImage image;//image original sin cambiar su tamaño
    QPixmap pixmap; //pixmap original sin cambiar su tamaño

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
    

  	float mouseMovementEventPeriod; // period between notifying two mouse movement events, in seconds.  
	unsigned long lastMouseMovementEvent;

private:
	float scale() const 
	{
		QSize tamOri  = pixmap.size();
		QSize tamDest = this->size();
		if ((tamOri.width()>0) && (tamDest.width()>0) && (tamOri.height()>0) && (tamDest.height()>0)) {
			if ((tamOri.height()/tamOri.width()) > (tamDest.height()/tamDest.width()))
				return float(tamDest.height())/float(tamOri.height());
			else
				return float(tamDest.width())/float(tamOri.width());
		}
		else return 0.0f;	
	}

	void updatePixmap() {
		float s = scale();
		if (s>0.0f) {
			QSize tamOri = pixmap.size();
			this->setPixmap(pixmap.scaled(int(s*float(tamOri.width())), int(s*float(tamOri.height())), Qt::KeepAspectRatio));
		}
	} 

	void updatePositionFromMouse(int x, int y)
	{
		float s = scale();
		_x = int(s*float(x));
		_y = int(s*float(y));
	}	

public:
    //explicit imageLabel(QWidget *parent = 0);
    LabelImage(QWidget* parent = 0) : QLabel(parent), mouseMovementEventPeriod(0.05), lastMouseMovementEvent(0)
    {
        this->setBackgroundRole(QPalette::Base);
	this->setMinimumSize(1,1);
    	this->setScaledContents(false);
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        this->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        this->setMouseTracking(false);
        //this->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    }

    void setMouseMovementEventPeriod(float p) { mouseMovementEventPeriod=p; }


    void setImage(const QPixmap& p)
    {
	pixmap=p;
	updatePixmap();
    }

    void setImage(const QImage& im)
    {
	if (!im.isNull()) {
		image = im;
		setImage(QPixmap::fromImage(image));

	}
    }

    void setImage(const QString& name)
    {
	setImage(QImage(name));
    }

    //init imagen
    bool initImageLabel(QString nameIm="", int w=640, int h=320 )
    {
         name = nameIm;
	 setImage(nameIm);
/*         image.load(nameIm);
         
         
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

         
        // this->setPixmap(QPixmap::fromImage(image));*/

//         if (image.isNull()) return false;
         return true;
    }

    bool initImageLabel(const QImage& imQT,QString nameIm="", int w=640, int h=320 )
    {
	setImage(imQT);
	return true;
/*
        name = nameIm;
        image = imQT.copy();
        
        pixmap = QPixmap::fromImage(image);
        //this->setPixmap(QPixmap::fromImage(image));
        this->setPixmap(pixmap);//.scaled(image.width(), image.height(),Qt::KeepAspectRatio));
        resize(sizeHint());//this->adjustSize();
        
        if (image.isNull()) return false;
        else return true;*/
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
	if ((!pixmap.isNull()) && (0.001f*float(event->timestamp() - lastMouseMovementEvent)>=mouseMovementEventPeriod)) {
		this->updatePositionFromMouse(event->x(), event->y());
		if (paint) drawStroke(event);
		emit mousePixelChanged(_x, _y);
		lastMouseMovementEvent=event->timestamp();
	} 
    }

    void mousePressEvent(QMouseEvent *event)
    {
	if ((!pixmap.isNull()) && (0.001f*float(event->timestamp() - lastMouseMovementEvent)>=mouseMovementEventPeriod)) {
		this->updatePositionFromMouse(event->x(), event->y());
                if (event->button()== Qt::LeftButton)
                {
                    enablePaint();
                    drawStroke(event);
                }
                else if (event->button()== Qt::RightButton)
                {
                    resetImage();
                    disablePaint();
                    //fprintf(stderr,"Antes de emitir la señal");
                    emit removeImageStrokes();
                }
        	emit mousePixelDown(_x, _y, event);
		emit mousePixelChanged(_x, _y);
		lastMouseMovementEvent=event->timestamp();
	} 
    }
    
    void mouseReleaseEvent(QMouseEvent *event)
    {
	if ((!pixmap.isNull()) && (0.001f*float(event->timestamp() - lastMouseMovementEvent)>=mouseMovementEventPeriod)) {
		this->updatePositionFromMouse(event->x(), event->y());
		disablePaint();
        	emit mousePixelUp(_x, _y, event);
		emit mousePixelChanged(_x, _y);
		lastMouseMovementEvent=event->timestamp();
	}
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
	    updatePixmap();
        }
    }
    
    void resizeEvent(QResizeEvent* event)
    {
	this->updatePixmap();
     /*   //calcular zoom ratio
        QSize tamOri = image.size();
        
        if (tamOri.width()!=0)
        {
			
             float newHeight = (float)tamOri.height()/(float)tamOri.width() * (float)parentWidget()->size().width();
           // float newWidht = (float)tamOri.width()/(float)tamOri.height() * (float)parentWidget()->size().height();
      
        
            this->setPixmap(pixmap.scaled(parentWidget()->size().width(), newHeight,Qt::KeepAspectRatio));
            // this->setPixmap(pixmap.scaled(newWidht, parentWidget()->size().height(),Qt::KeepAspectRatio));
        
            this->adjustSize();
        }
        //fprintf(stderr,"resize label %d %d\n",image.width(),image.height());*/
    }

//señales a emitir y capturar desde fuera
signals:
    void mousePixelChanged(int x, int y);
    void mousePixelDown(int x, int y,QMouseEvent *event);
    void mousePixelUp(int x, int y,QMouseEvent *event);
    void removeImageStrokes();
    
};

#endif // LABEL_IMAGE
