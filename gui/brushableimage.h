#pragma once

#include "labelimage.h"

class BrushableImage : public QObject
{
    	Q_OBJECT
private:
	LabelImage* connectedTo;
    	QImage  image;   //original image
	QPixmap pixmap;  //pixmap with painted strokes

	bool canEdit;
	bool paint;
	QColor colorBlush;
	QPainter painter;

public:
    BrushableImage() : 
	    connectedTo(nullptr), canEdit(false), paint(false)
    {
    
    }

    void clear()
    {
	if (!image.isNull()) {
		pixmap = QPixmap::fromImage(image);
	}
    }

    void setImage(const QImage& im)
    {
	if (!im.isNull()) {
		image = im;
		clear();

	}
    }

    //DRAW over image
    void setColorBrush(QColor color)
    {
        //cambiar color del pixel del raton
        colorBlush = color;
	canEdit    = true;
    }

    void setColorBlush(int r, int g, int b)
    {
	    setColorBrush(QColor::fromRgb(r,g,b));
    }

    void unsetBrush()
    {
	canEdit=false;
    }

    void connectToLabelImage(LabelImage* li, bool connected = true)
    {
	if (li) {
		QObject::connect(li, &LabelImage::mousePixelDown,
                         this, &BrushableImage::pixelDown);
		QObject::connect(li, &LabelImage::mousePixelUp,
                         this, &BrushableImage::pixelUp);
		QObject::connect(li, &LabelImage::mousePixelChanged,
                         this, &BrushableImage::pixelMoving);
        
		if (connected)
			connectedTo = li;
	}

    }

    void pixelMoving(int x, int y)
    {
	if (canEdit && paint && (!image.isNull())) {
            QPainter painter(&pixmap);
            painter.setBrush(QBrush(colorBlush));
            
            QSize tamOri = image.size();
            painter.drawEllipse(x, y,0.025*tamOri.width(),0.025*tamOri.width());
	    if (connectedTo)
		    connectedTo->setImage(pixmap);
	}
    }

    void pixelUp(int x, int y,QMouseEvent *event)
    {
	paint = false;
    }

    void pixelDown(int x, int y,QMouseEvent *event)
    {
    	if (event->button()== Qt::LeftButton) {
		paint = true;
	} else if (event->button() == Qt::RightButton) {
		clear();
	}
    }

};


