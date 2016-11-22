#ifndef WIDGETIMAGE_H
#define WIDGETIMAGE_H

#include <QWidget>
#include <QScrollArea>
#include <QApplication>
#include <QLabel>
#include <QSlider>
#include "gridButtons.h"
#include <gui/labelimage.h>
#include <gui/imagemousebrush.h>
#include <gui/multiimageviewer.h>


#include "denseLabeling.cpp"
#include "dehazing/dehazing.h"

#include "imageHDR.h"

#include <stdio.h>

#define DEPTH_NEAR 224.0
#define DEPTH_FAR_1 160.0
#define DEPTH_FAR_2 96.0
#define DEPTH_FAR_3 32.0

#define NO_FOG 255.0
#define MIN_FOG 160.0
#define MEDIUM_FOG 96.0
#define MAX_FOG 30.0

class WidgetImage : public QWidget
{
    Q_OBJECT
    
private:
	    std::shared_ptr<QPixmap> strokes_pixmap;
	    std::shared_ptr<cv::Mat> input_image;
	    std::shared_ptr<cv::Mat> filtered_image;
	    std::shared_ptr<cv::Mat> propagated_channel;
    
    //interfaz
    GridButtons     *buttonOptions;
    ImageMouseBrush *imageMouseBrush;
    MultiImageViewer *multiImageViewer;

    int buttonIdStrokes, buttonIdInput, buttonIdFiltered, buttonIdChannel;
    
    QLabel* info = new QLabel();
    
    //Datos
    DenseLabeling *denseDepth = NULL;
    
    //valor de profundidad
    float valueDepth = 255.0;
    double _minFocus = 220, _maxFocus=0.0;
    double _sizeFocus = 50.0;
    
public:
    
    //construir la interfaz
    WidgetImage() :
	    strokes_pixmap(std::make_shared<QPixmap>()), input_image(std::make_shared<cv::Mat>()), filtered_image(std::make_shared<cv::Mat>()),propagated_channel(std::make_shared<cv::Mat>()),
	    imageMouseBrush(new ImageMouseBrush(input_image, strokes_pixmap)), multiImageViewer(new MultiImageViewer())
    {
        QHBoxLayout *layoutH = new QHBoxLayout;
        this->setWindowTitle("App Depth-of-Field simulation");
        this-> setMinimumSize(400,400);
        
        QVBoxLayout *buttons = new QVBoxLayout;
        
        QGroupBox *boxOptions;
        boxOptions = new QGroupBox("Select options:");
        boxOptions->setMinimumSize(30,320);//(180,320);
        
        buttonOptions = new GridButtons( boxOptions );
        
        QSlider* sliderFocus;
        sliderFocus = new QSlider(Qt::Horizontal);
        
        sliderFocus->setTickPosition(QSlider::TicksAbove);
        // sliderFocus->setTickInterval(100);
        sliderFocus->setRange(1,100);
        sliderFocus->setValue(50);
        sliderFocus->setEnabled(true);
        sliderFocus->setFixedSize(180,20);
        
        buttons->addWidget(boxOptions);
        buttons->addStretch();
        buttons->addWidget(sliderFocus);
        
        layoutH->addLayout(buttons);
        
//        QScrollArea *boxImage = new QScrollArea;
        
 //       boxImage->setWidget(imageToEdit);
        
        QVBoxLayout *image = new QVBoxLayout;
        image->addWidget(multiImageViewer);
        
        
        //add label de info
        info->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        info->setFixedSize(640,20);
        image->addWidget(info);
        
        layoutH->addLayout(image);
        setLayout(layoutH);

	imageMouseBrush->connectToLabelImage(multiImageViewer->labelImage());
	buttonIdInput    = multiImageViewer->add("Input",input_image);
	buttonIdStrokes  = multiImageViewer->add("Strokes",imageMouseBrush->getPixmap());
	buttonIdFiltered = multiImageViewer->add("Filtered",filtered_image);
	buttonIdChannel = multiImageViewer->add("Transmittance",propagated_channel);

        
        
        //conectar señales
        //click sobre opcion, cambiar color pincel
        QObject::connect(buttonOptions,  &GridButtons::optionSelected,
                         this,&WidgetImage::changeColorPaint);
        
        QObject::connect(multiImageViewer->labelImage(), &LabelImage::mousePixelDown,
                         this, &WidgetImage::clickMousePixel);
        QObject::connect(multiImageViewer->labelImage(), &LabelImage::mousePixelUp,
                         this, &WidgetImage::unclickMousePixel);
        
        QObject::connect(multiImageViewer->labelImage(), &LabelImage::mousePixelChanged,
                         this, &WidgetImage::updatePixel);
        
        //cambiar tamaño focus
        QObject::connect(sliderFocus, SIGNAL(valueChanged(int)), this, SLOT(updateSizeFocus(int)));
        
        resize(sizeHint());
    }
    
    void setInfo(QString sms)
    {
        info->setText(sms);
        QApplication::processEvents();
    }
    
    void resizeEvent(QResizeEvent* event)
    {
        // imageToEdit->resizeEvent(event);
        
        if (denseDepth != NULL )
            processImage();
    }
    
    
    public slots:
    void changeColorPaint(int id)
    {
        switch (id)
        {
            case ID_1:
                imageMouseBrush->setColorBrush(QColor::fromRgb(0, 0,255));
                this->setCursor(Qt::PointingHandCursor);
                valueDepth=NO_FOG;//224;//250.0;
                // imageToEdit->setCanEdit(true);
                break;
            case ID_2:
                imageMouseBrush->setColorBrush(QColor::fromRgb(0, 255,0));
                this->setCursor(Qt::PointingHandCursor);
                valueDepth=MIN_FOG;//160;//150.0;
                //  imageToEdit->setCanEdit(true);
                break;
            case ID_3:
                imageMouseBrush->setColorBrush(QColor::fromRgb(0, 150,0));
                this->setCursor(Qt::PointingHandCursor);
                valueDepth=MEDIUM_FOG;//96;//50.0;
                //  imageToEdit->setCanEdit(true);
                break;
            case ID_4:
                imageMouseBrush->setColorBrush(QColor::fromRgb(0, 75,0));
                this->setCursor(Qt::PointingHandCursor);
                valueDepth=MAX_FOG;//32;//10.0;
                //   imageToEdit->setCanEdit(true);
                break;
                
            /*case ID_FOCUS:
                imageToEdit->setColorBlush(QColor::fromRgb(255, 0,0));
                //   imageToEdit->setCanEdit(false);
                // this->setCursor(Qt::PointingHandCursor);
                processImage();
                break;*/
                
        }
        // fprintf(stderr,"cambiar color %d \n",id);
    }
    
    void resetOptions()
    {
        this->setCursor(Qt::ArrowCursor);
        _minFocus = 250.0;
        _maxFocus = 255.0;
        destroyAllWindows();
        //buttonOptions->resetSelection();
    }
    
    void updateSizeFocus(int size)
    {
        _sizeFocus = size;//double (size)*3.0;
        processImage();
    }
    
    //////////////// SUPERPIXELS + BINARY EQUATIONS
    bool loadData(const QString& filename)
    {
	    load_image(filename.toStdString().c_str(), input_image);
	    imageMouseBrush->clear();
	    *filtered_image = *input_image;

            //mostrar imagen en la interfaz
	    multiImageViewer->setButton(buttonIdInput);
//            imageToEdit->set(input_image);
//            imageMouseBrush->setImage(input_image);

        denseDepth =  new DenseLabeling(filename.toStdString(),0.3,0.99,10.0);
        
        if (!denseDepth->isNotNullImage())
        {
            setInfo("Problem to open the image!");
            return false;
        }
        else
        {
            setInfo("Image opened correctly.");
        }
        //binary equations
        denseDepth->addEquations_BinariesBoundariesPerPixelMean();
       // denseDepth->addEquations_BinariesBoundariesPerPixel();
        setInfo("Binary equations created.");
        
        return true;
    }

    //////////////// INPUT UNARY EQUATIONS
    void loadDepth(QString filename)
    {
        denseDepth->addEquations_Unaries(filename.toStdString());
        setInfo("Load input depth (all unary equations)");
        
        processImage();
        setInfo("System Solved.");
    }
    
    void saveData(QString dir)
    {
        processImage(true,dir.toStdString());
        setInfo("All files Saved.");
    }
    
    ////////////////
    
    //////// MOUSE
    void updatePixel(int x, int y)
    {
        if (input_image)
        {
            setInfo("Add new unary equation");
            denseDepth->addEquation_Unary(x,y,valueDepth/255.0);
	    multiImageViewer->setButton(buttonIdStrokes);
        }
    }
    
    void clickMousePixel(int x, int y,QMouseEvent * event)
    {
        if (event->button() == Qt::LeftButton)
        {
            if (input_image)
            {
                //imageToEdit->enablePaint();
                //si blur activado: cambiar color
                if (buttonOptions->focusSelected())
                {
                    updateFocus(x,y);
                }
                else
                {
                    setInfo("Add new unary equation");
                    denseDepth->addEquation_Unary(x,y,valueDepth/255.0);
                }
            }
	    multiImageViewer->setButton(buttonIdStrokes);
        }
    }
    
    void unclickMousePixel(int x, int y,QMouseEvent * event)
    {
        // if (imageToEdit->getCanEdit())        
        processImage();
    }
    
    void updateFocus(int x, int y)
    {
        double newDepth = denseDepth->getLabel(x,y) * 255.0;
        
        //if (newDepth < _minFocus)
            _minFocus = newDepth;
       // if (newDepth > _maxFocus)
         //   _maxFocus = newDepth;
        
         //fprintf(stderr,"_min: %f max: %f new: %f",_minFocus,_maxFocus,newDepth);
    }
    
    void processImage(bool save=false, string dir = "")
    {

        //show denseDepth estimation
        *propagated_channel = denseDepth->solve();
        
        
        double min, max;
	cv::minMaxLoc(*propagated_channel, &min, &max);
        
        
        
        //DEHAZ
        double min_t = ((_sizeFocus/100.0)*min) + ((1.0 - (_sizeFocus/100.0))*max);//0.05;
        double max_t = max;//0.95;
        
//      printf("Mat sol: min %f max %f slider %f MIN_T %f \n",min,max,_sizeFocus,min_t);
        
        *filtered_image = dehaze(*input_image, *propagated_channel, min_t, max_t);
        
        //BLUR
        /*int   nbins          = 8;
        float aperture       = _sizeFocus;
        float focal_distance = _minFocus;
        float focal_length   = _minFocus+20;
        bool  linear         = true;
        Mat final = blur_image_depth(denseDepth->getImage(), sol_gray, nbins,focal_distance,focal_length,aperture, linear);
        */
        //
        
        if (save)
        {
        	Mat sol_gray = (*propagated_channel) * 255.0;
        	sol_gray.convertTo(sol_gray,CV_8UC1);
            Mat user = denseDepth->getImageLabelsInput() * 255.0;
            user.convertTo(user, CV_8UC1);            
            string name = dir + "/user_input.png";
            setInfo("Save images");
            imwrite(name,user);
            name = dir + "/dehaz.png";
            imwrite(name,*filtered_image);
            name = dir + "/depth.png";
            imwrite(name,sol_gray);
        }
//        cv::resize(sol_gray, sol_gray, Size(imageToEdit->size().width(),imageToEdit->size().height()));
//       imshow("solution",sol_gray);
//        cv::resize(final, final, Size(imageToEdit->size().width(),imageToEdit->size().height()));
        
        //imshow("dehaz",final);
        
	
	multiImageViewer->setButton(buttonIdFiltered);
//        if (buttonOptions->idSelected() != -1)
//            imageToEdit->set(filtered_image);//*/
    }
};

#endif
