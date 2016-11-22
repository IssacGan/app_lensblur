#pragma once

#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QMouseEvent>
#include "imageformats.h"

class LabelImage : public QLabel
{
    Q_OBJECT
private:
    QPixmap pixmap; //pixmap original sin cambiar su tamaño

     //pixel selected
    int _x=0;
    int _y=0;
    
  	float mouseMovementEventPeriod; // period between notifying two mouse movement events, in seconds.  
	unsigned long lastMouseMovementEvent;

private:
	float scale() const 
	{
		QSize tamOri  = pixmap.size();
		QSize tamDest = this->size();
		if ((tamOri.width()>0) && (tamDest.width()>0) && (tamOri.height()>0) && (tamDest.height()>0)) {
			return std::min(float(tamDest.height())/float(tamOri.height()),
					float(tamDest.width())/float(tamOri.width()));
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
		_x = int(float(x)/s);
		_y = int(float(y)/s);
		if (_x>=pixmap.width()) _x=pixmap.width()-1;
		if (_x<0) _x=0;
		if (_y>=pixmap.height()) _y=pixmap.height()-1;
		if (_y<0) _y=0;
	}	

public:
    //explicit imageLabel(QWidget *parent = 0);
    LabelImage(QWidget* parent = 0) : QLabel(parent), mouseMovementEventPeriod(0.01), lastMouseMovementEvent(0)
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


    void set(const QPixmap& p)
    {
	pixmap=p;
	updatePixmap();
    }

    void set(const std::shared_ptr<cv::Mat>& image)
    {	
	    if (image) set(qpixmap(*image));
    }	    

    bool editable() const { return !pixmap.isNull(); }


    //Eventos MOUSE
    void mouseMoveEvent(QMouseEvent *event)
    {
	if ((!pixmap.isNull()) && (0.001f*float(event->timestamp() - lastMouseMovementEvent)>=mouseMovementEventPeriod)) {
		this->updatePositionFromMouse(event->x(), event->y());
		emit mousePixelChanged(_x, _y);
		lastMouseMovementEvent=event->timestamp();
	} 
    }

    void mousePressEvent(QMouseEvent *event)
    {
	if (!pixmap.isNull()) {
		this->updatePositionFromMouse(event->x(), event->y());
        	emit mousePixelDown(_x, _y, event);
		emit mousePixelChanged(_x, _y);
		lastMouseMovementEvent=event->timestamp();
	} 
    }
    
    void mouseReleaseEvent(QMouseEvent *event)
    {
	if (!pixmap.isNull()) {
		this->updatePositionFromMouse(event->x(), event->y());
		emit mousePixelChanged(_x, _y);
        	emit mousePixelUp(_x, _y, event);
		lastMouseMovementEvent=event->timestamp();
	}
    }	

    void resizeEvent(QResizeEvent* event)
    {
	this->updatePixmap();
    }

//señales a emitir y capturar desde fuera
signals:
    void mousePixelChanged(int x, int y);
    void mousePixelDown(int x, int y,QMouseEvent *event);
    void mousePixelUp(int x, int y,QMouseEvent *event);
};

