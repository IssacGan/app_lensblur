#ifndef WIDGETFOCUS_H
#define WIDGETFOCUS_H

#include <QWidget>
#include <QScrollArea>
#include <QApplication>
#include <QLabel>
#include <QSlider>
#include "gridbuttons.h"
#include "labelimage.h"

#include "Nsuperpixels/superpixels.h"
#include <stdio.h>

class WidgetFocus : public QWidget
{
    Q_OBJECT
    
private:
    
    //interfaz
    GridButtons *buttonOptions;
    LabelImage *imageToEdit= new LabelImage();
    
    QLabel* info = new QLabel();
    
    
    //Datos
    SuperPixels *sp;
    SuperPixels *spCTE;
    
    //valor de profundidad
    float valueDepth = 255.0;
    double _minFocus = 220, _maxFocus=0.0;
    double _sizeFocus = 7.0;
    
public:
    
    //construir la interfaz
    WidgetFocus()
    {
        QHBoxLayout *layoutH = new QHBoxLayout;
        this->setWindowTitle("App focus/defocus with depth");
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
        sliderFocus->setValue(25);
        sliderFocus->setEnabled(true);
        sliderFocus->setFixedSize(180,20);
        
        buttons->addWidget(boxOptions);
        buttons->addStretch();
        buttons->addWidget(sliderFocus);
        
        layoutH->addLayout(buttons);
        
        
        QScrollArea *boxImage = new QScrollArea;
        
        
        boxImage->setWidget(imageToEdit);
        
        QVBoxLayout *image = new QVBoxLayout;
        image -> addWidget(boxImage);
        
        
        //add label de info
        info->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        info->setFixedSize(640,20);
        image -> addWidget(info);
        
        layoutH->addLayout(image);
        setLayout(layoutH);
        
        
        //conectar señales
        //click sobre opcion, cambiar color pincel
        QObject::connect(buttonOptions,  &GridButtons::optionSelected,
                         this,&WidgetFocus::changeColorPaint);//*/
        
        
        QObject::connect(imageToEdit, &LabelImage::mousePixelDown,
                         this, &WidgetFocus::clickMousePixel);
        QObject::connect(imageToEdit, &LabelImage::mousePixelUp,
                         this, &WidgetFocus::unclickMousePixel);
        
        QObject::connect(imageToEdit, &LabelImage::mousePixelChanged,
                         this, &WidgetFocus::updatePixel);
        
        //cambiar tamaño focus
        QObject::connect(sliderFocus, SIGNAL(valueChanged(int)), this, SLOT(updateSizeFocus(int)));
        
        resize(sizeHint());
        /* QObject::connect(imageToEdit,
         &ImageLabel::removeImageStrokes,
         this,&WidgetFocus::resetOptions);//*/
        
    }
    
    ~WidgetFocus() {};
    
    void setInfo(QString sms)
    {
        info->setText(sms);
        QApplication::processEvents();
    }
    
    void resizeEvent(QResizeEvent* event)
    {
        
        imageToEdit->resizeEvent(event);
        //fprintf(stderr,"RESIZE widget focus %d %d \n",this->size().width(),this->size().height());
    }
    
    
    public slots:
    void changeColorPaint(int id)
    {
        
        switch (id)
        {
            case ID_NEAR:
                imageToEdit->setColorBlush(QColor::fromRgb(0, 0,255));
                this->setCursor(Qt::PointingHandCursor);
                valueDepth=DEPTH_NEAR;//224;//250.0;
                // imageToEdit->setCanEdit(true);
                break;
            case ID_FAR_1:
                imageToEdit->setColorBlush(QColor::fromRgb(0, 255,0));
                this->setCursor(Qt::PointingHandCursor);
                valueDepth=DEPTH_FAR_1;//160;//150.0;
                //  imageToEdit->setCanEdit(true);
                break;
            case ID_FAR_2:
                imageToEdit->setColorBlush(QColor::fromRgb(0, 150,0));
                this->setCursor(Qt::PointingHandCursor);
                valueDepth=DEPTH_FAR_2;//96;//50.0;
                //  imageToEdit->setCanEdit(true);
                break;
            case ID_FAR_3:
                imageToEdit->setColorBlush(QColor::fromRgb(0, 75,0));
                this->setCursor(Qt::PointingHandCursor);
                valueDepth=DEPTH_FAR_3;//32;//10.0;
                //   imageToEdit->setCanEdit(true);
                break;
                
            case ID_FOCUS:
                imageToEdit->setColorBlush(QColor::fromRgb(255, 0,0));
                //   imageToEdit->setCanEdit(false);
                // this->setCursor(Qt::PointingHandCursor);
                blurImage();
                break;
                
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
        blurImage();
        
        //cout << "Focus: "<< size << "   size: "<< _sizeFocus << endl;
    }
    
    //////////////// SUPERPIXELS
    
    bool loadData(QString filename)
    {
        
        imageToEdit->setCanEdit(false);
        //cargar imagen original
        loadImage(filename);
        
        QString superpixels = filename.left(filename.lastIndexOf("/"));
        QString depth = superpixels.left(superpixels.lastIndexOf("/")) + "/depth";
        
        QString name = filename.right(filename.length() - filename.lastIndexOf("/") - 1);
        QString aux = name.left(name.lastIndexOf("."));
        
        //cargar los superpixels
        superpixels = superpixels + "/superpixels/" + aux + "_SLIC_30.sp";
        loadSuperPixels(superpixels);
        //  imshow("Superpixels",spCTE->getImage());
        
        //cargar depth
        loadDepth();
        
        
        if (!sp->isNotNullImage() || !sp->isNotNullDepth() || !sp->isNotNullIndex())
        {
            setInfo("Error while load data");
            return false;
        }
        else
        {
            setInfo("Data loaded successfully");
            imageToEdit->setCanEdit(true);
            return true;
        }
        
        //
        
        
        
    }
    
    void loadImage(QString filename)
    {
        bool opened=false;
        
        if (!filename.isEmpty())
        {
            //mostrar imagen en la interfaz
            opened = imageToEdit->initImageLabel(filename,640,320);
            
            //inicializar superpixels
            sp = new SuperPixels(filename.toStdString());
            
            //
            spCTE = new SuperPixels(filename.toStdString());
        }
        
        //comprobar si se ha abierto la imagen y los superpixels
        if (!sp->isNotNullImage() || !opened )
            setInfo("Problem to open the image");
        else
            setInfo("Image opened correctly.");
        
    }
    
    void loadSuperPixels(QString filename)
    {
        sp->loadSuperPixels(filename.toStdString());
        setInfo("Superpixels file loaded");
        
        spCTE->loadSuperPixels(filename.toStdString());
    }
    
    void loadDepth()
    {
        setInfo("Loading depth, build system...");
        sp->loadDepthBuildSystemCoef("");
        sp->addEquationsBinariesBoundariesCoef();
        setInfo("Depth file loaded");
        
        spCTE->loadDepthBuildSystem("");
        
    }
    
    void saveData()
    {
        //guardar depth y
        //guardar la image de defocus
    }
    
    ////////////////
    
    //////// MOUSE
    void updatePixel(int x, int y)
    {
        /* std::stringstream sstr;
         sstr<<std::setw(7)<<"x: "<<  x <<" y: "<< y;
         setInfo(QString(sstr.str().c_str()));*/
        
        if (buttonOptions->focusSelected())
            imageToEdit->disablePaint();
        else if (imageToEdit->getCanEdit())
        {
            //sp->addEquationsUnaries(sp->getIdFromPixel(x,y), valueDepth/255.0);
            clock_t start = clock();
            
            
            sp->addUnariesCoef( x,  y,  valueDepth/255.0);
            
            printf("**** TIEMPO: Ecuacion unaria: %f seconds\n ",(float) (((double)(clock() - start)) / CLOCKS_PER_SEC) );
            
            //CTE
            spCTE->addEquationsUnaries(x,y, valueDepth/255.0 );
            
        }
        
        //fprintf(stderr,"PIXEL %d %d \n",x,y);
        
    }
    
    void clickMousePixel(int x, int y,QMouseEvent * event)
    {
        // fprintf(stderr,"PIXEL %d %d \n",x,y);
        if (event->button() == Qt::LeftButton)
        {
            if (imageToEdit->getCanEdit())
            {
                //imageToEdit->enablePaint();
                //si blur activado: cambiar color
                if (buttonOptions->focusSelected())
                {
                    updateFocus(x,y);
                }
                else
                {
                    
                    clock_t start = clock();
                    
                    sp->addUnariesCoef(x,y, valueDepth/255.0);
                    
                    printf("**** TIEMPO: Ecuacion unaria: %f seconds\n ",(float) (((double)(clock() - start)) / CLOCKS_PER_SEC) );
                    
                    //cTE
                    spCTE->addEquationsUnaries(sp->getIdFromPixel(x,y), valueDepth/255.0);
                    
                }
                
                
                //sp->addEquationsUnaries(sp->getIdFromPixel(x,y), valueDepth/255.0);
                // sp->addEquationsUnaries(x,y, valueDepth/255.0);
            }
        }
    }
    
    void unclickMousePixel(int x, int y,QMouseEvent * event)
    {
        // if (imageToEdit->getCanEdit())
        
        blurImage();
        
        if (buttonOptions->focusSelected())
            imageToEdit->enablePaint();
    }
    
    void updateFocus(int x, int y)
    {
        double newDepth = sp->getDepthInitialFromPixel(x,y) * 255.0;
        
        //   if (newDepth < _minFocus)
        _minFocus = newDepth;
        /* if (newDepth > _maxFocus)
         _maxFocus = newDepth;*/
        //   _maxFocus=_minFocus+50;
        
        // fprintf(stderr,"_min: %f max: %f new: %f",_minFocus,_maxFocus,newDepth);
    }
    
    void blurImage()
    {
        clock_t start = clock();
        
        sp->solveCoef();
        
        printf("**** TIEMPO: Solve: %f seconds\n ",(float) (((double)(clock() - start)) / CLOCKS_PER_SEC) );
        
        
        Mat f=sp->getImage();
        // imwrite("depth.png", f);
        
        cv::resize(f, f, Size(imageToEdit->size().width(),imageToEdit->size().height()));
        imshow("depth",f);
        
        
        start = clock();
        
        //BLUR
       /* Mat image;
        cvtColor(sp->_lab,image,CV_Lab2BGR);
        Mat imageDepth = sp->_pixelDepth*255.0;
        
        Mat final;
        
        
        int   nbins          = 8;
        float aperture       = _sizeFocus;
        float focal_distance = _minFocus;
        float focal_length   = _minFocus+20;
        bool  linear         = true;
        Mat b;
        //  if (_minFocus == 255.0 && _maxFocus == 0.0)
        //      b=  sp->blurImage(image, imageDepth, nbins,_minFocus,_minFocus+25,_sizeFocus);
        // else
        // double max = _minFocus+20 > 255 ? 255 : _minFocus+20;
        // b=  sp->blurImage(image, imageDepth, nbins,_minFocus,max,_sizeFocus);
        b=sp->blurImageDepth(image, imageDepth, nbins,focal_distance,focal_length,aperture, linear);
        // Mat b=  sp->blurImage(image, imageDepth, nbins,255.0,255.0,_sizeFocus);
        //sp->blurImage(image, imageDepth, nbins);
        //cvtColor(b,b,CV_RGB2BGR);
        cv::resize(b, b, Size(imageToEdit->size().width(),imageToEdit->size().height()));
        //imshow("Blur",b);
        //imwrite("blur.png", b);

        printf("**** TIEMPO: Blur: %f seconds\n ",(float) (((double)(clock() - start)) / CLOCKS_PER_SEC) );
        
        cvtColor(b,b,CV_BGR2RGB);
        
        // if (buttonOptions->focusSelected())
        imageToEdit->setImage(convertQtImage(b));*/
        
        
        //CTE
        spCTE->solve();
        
        Mat fcte=spCTE->getImage();
        // imwrite("depth.png", f);
        
        cv::resize(fcte, fcte, Size(imageToEdit->size().width(),imageToEdit->size().height()));
        imshow("depth CTE",fcte);
    }
    
    QImage convertQtImage(Mat _image)
    {
        //QImage point to the data of _image
        return QImage(_image.data, _image.cols, _image.rows, _image.step, QImage::Format_RGB888);
    }
    
    
};

#endif // WIDGETFOCUS_H
