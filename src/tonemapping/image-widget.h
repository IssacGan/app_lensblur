#ifndef _IMAGE_WIDGET_H_
#define _IMAGE_WIDGET_H_

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollBar>
#include <QSpinBox>
#include <QAbstractSpinBox>
#include <QSlider>
#include <QSplitter>
#include <QCheckBox>
#include <QPainter>


#include <QScrollArea>
#include <QAbstractScrollArea>

#include <QPushButton>
#include <QProgressBar>

#include <QApplication>

#include<iostream>
#include<sstream>
#include<iomanip>
#include<stdio.h>

#include "superpixels/superpixels.cpp"
#include "imagelabel.h"


class ImageWidget : public QWidget
{
    Q_OBJECT

private:
    //imagen a etiquetar
   // ImageLabel*
    QLabel* imageLabel;
    SuperPixels *sp;

    QScrollArea *scrollArea;
    int pixel_x, pixel_y;
    int last_pixel_x, last_pixel_y;

    QLabel*     zoomLabel;
    QLabel*     pixelLabel;
    QLabel*     info;

    //QLabel* original;
    ImageLabel* original;

    QLabel* colorSelected;
    QSpinBox* spinner;
    QSlider* slider;
    QProgressBar* bar;

    bool selection=false;
    
    bool AddEq=false;
    
    QCheckBox *checkbox;
    
    QCheckBox *checkboxBoundary;
    //peso de las ecuaciones unarias frente a las binarias
    QSpinBox* spinner_w_u;
    
    //button del blur
    QPushButton* buttonEffect;
    QSlider* sliderFocus;
    
    double _sizeFocus=1.0;
    
    //focus
    double _minFocus = 255.0, _maxFocus=0.0;

    void updateZoom()
    {
        /*std::stringstream sstr;
        sstr<<std::fixed<<std::setprecision(2)<<std::setw(8)<<imageLabel->getZoom()*100.0<<"\%";
        zoomLabel->setText(QString(sstr.str().c_str()));*/
        std::stringstream sstr;
        sstr<<std::fixed<<std::setprecision(2)<<std::setw(8)<<original->getZoom()*100.0<<"\%";
        zoomLabel->setText(QString(sstr.str().c_str()));
        
    }
    
    void updateFocus()
    {
        /*double newDepth = sp->getDepthInitialFromPixel(pixel_x, pixel_y) * 255;
        
        if (newDepth < _minFocus)
            _minFocus = newDepth;
        if (newDepth > _maxFocus)
            _maxFocus = newDepth;*/
        
        cout << "FOCUS: " << _minFocus<< " "<< _maxFocus<<endl;

    
    }

    void updatePixel()
    {
        std::stringstream sstr;
        sstr<<std::setw(7)<<pixel_x<<", "<<pixel_y;
        pixelLabel->setText(QString(sstr.str().c_str()));
        
        if (selection)
            updateFocus();
       /* else
            _minFocus = 255.0, _maxFocus=0.0;*/
    }

public:
   
    void mousePixelChanged(int x, int y){
        cout << "image-widget: "<< x << " , "<< y<< endl;
    
    }
    

    void setZoom(double factor = 1.0)
    {
       // imageLabel->setZoom(factor);
        original->setZoom(factor);
        updateZoom();
    }


    void zoom(double factor)
    {
     //   imageLabel->zoom(factor);
        original->setZoom(factor);
        
        updateZoom();
    }

    bool loadData(QString filename)
    {
        //update progressBar
        bar->setVisible(true);
        bar->setRange(0,0);
        bar->setValue(1);

        //cargar imagen original
        loadImage(filename);

        QString superpixels = filename.left(filename.lastIndexOf("/"));
        QString depth = superpixels.left(superpixels.lastIndexOf("/")) + "/depth";

        QString name = filename.right(filename.length() - filename.lastIndexOf("/") - 1);
        QString aux = name.left(name.lastIndexOf("."));

        //cargar los superpixels
        superpixels = superpixels + "/superpixels/" + aux + "_SLIC_30.sp";
        loadSuperPixels(superpixels); 
        //loadSuperPixels("/Users/acambra/TESIS/DepthSynthesis/appGUI_Xcode/Debug/IMG_1947.dat");

        //cargar depth
       // depth = depth + "/depth_" + aux + ".png";
        //loadDepth(depth);
        if (checkbox->checkState()== Qt::CheckState::Checked)
        {
        }
        else{
            //histogramas
            loadDepthBuildSystem();
            
        }
        
    


        //int num = aux.toInt();

        if (!sp->isNotNullImage() )//|| !sp->isNotNullDepth() || !sp->isNotNullIndex())
        {
            setInfo("Error while load data");
            return false;
        }
        else
        {
            setInfo("Data loaded successfully");
            return true;
        }
        
        
    }

    void loadImage(QString filename)
    {
        sp = new SuperPixels(filename.toStdString());

        //test
        //sp-> loadSuperPixels("/Users/anacambra/Desktop/shareWin/IMG_1939.dat");
        if (!filename.isEmpty())
        {
            QImage image(filename);
            //image = image.scaledToWidth(this->size().width()/2-50);
            //original->setPixmap(QPixmap::fromImage(image));
            original->setImage(image);
            original->setOriginal(image);
            zoomFit();
            setInfo("Open image");
        }
        
    }

    void loadSuperPixels(QString filename)
    {
       sp->loadSuperPixels(filename.toStdString());
       setInfo("Superpixels file loaded");
    }

    void loadDepth(QString filename)
    {
        setInfo("Loading depth...");
        //sp->loadDepth(filename.toStdString());
        setImage(convertQtImage(sp->getImage()));
        zoomFit();
        setInfo("Depth file loaded");

    }
    void loadDepthBuildSystem()
    {
        setInfo("Loading depth, build system...");
        //sp->loadDepthBuildSystem("");
        //setImage(convertQtImage(sp->getImage()));
        //imageLabel->setPixmap(QPixmap::fromImage(convertQtImage(sp->getImage())));
        zoomFit();
        setInfo("Depth file loaded");
        
    }

    void setImage(const QImage& qimage)
    {
      //  int h=scrollArea->horizontalScrollBar()->value();
      //  int v=scrollArea->verticalScrollBar()->value();
        //double f=original->getZoom();//imageLabel->getZoom();
        
        
       //TODO: cout << "size: w: " <<  imageLabel->size().width() << "h: " <<  imageLabel->size().height()<< endl;
        

       // imageLabel->setImage(qimage);
        imageLabel->setSizePolicy(QSizePolicy::Expanding,
                                        QSizePolicy::Expanding);
        imageLabel->setAlignment(Qt::AlignCenter);
        imageLabel->setMinimumSize(qimage.width(),qimage.height());
        
        imageLabel->setPixmap(QPixmap::fromImage(qimage));
        imageLabel-> setScaledContents(true);
       
      //  updateZoom();

       // zoom(f);
     /*   scrollArea->horizontalScrollBar()->setValue(h);
        scrollArea->verticalScrollBar()->setValue(v);*/

    }

    float paintSuperpixel()
    {
        std::stringstream sstr;
        sstr<<std::setw(7)<<"Superpixel id=";//<<  sp->getIdFromPixel(pixel_x,pixel_y) <<" DEPTH="<< sp->getDepthFromPixel(pixel_x,pixel_y);
        setInfo(QString(sstr.str().c_str()));

        /*Mat im=sp->paintSuperpixel(pixel_x,pixel_y, new cv::Scalar(255,0,0));
        
        if (last_pixel_x != -1 && last_pixel_y != -1)
        {
            im=sp->paintSuperpixel(last_pixel_x,last_pixel_y,new cv::Scalar(255,0,0));
            imshow("paintSelected",im);
            waitKey(1);
        }*/
        
       
        
        
       // printf("%f %f\n",sp->getDepthFromPixel(pixel_x,pixel_y),sp->arraySP[sp->getIdFromPixel(pixel_x,pixel_y)].d_median);
        
        return 0;//sp->getDepthFromPixel(pixel_x,pixel_y);
    }

    void repaintSuperpixel()
    {
        if (last_pixel_x == -1 && last_pixel_y == -1)
        {
            //not pixel selected
            float depth = (float) slider->value();
            //sp->copyDepth(pixel_x,pixel_y,depth/255);

        }else{

            if ( sp->getIdFromPixel(pixel_x, pixel_y) != sp->getIdFromPixel(last_pixel_x,last_pixel_y))
            {
                //sp->copySuperpixel(pixel_x,pixel_y,last_pixel_x,last_pixel_y);
                std::stringstream sstr;
                sstr<<std::setw(7)<<"COPIAR id="<<  sp->getIdFromPixel(pixel_x,pixel_y);// <<" DEPTH="<< sp->getDepthFromPixel(pixel_x,pixel_y);
                setInfo(QString(sstr.str().c_str()));
            }
        }
    }

    
public slots:
    void setInfo(QString sms)
    {
        info->setText(sms);
        QApplication::processEvents();
    }
    
    void setMousePixel(int x, int y)
    {    
       
        pixel_x = x; pixel_y = y;
        updatePixel();
        if (AddEq)
        {
            if (sp != 0 )
            {
                //sp->addEquationsUnaries(sp->getIdFromPixel(x, y), (float) slider->value()/255.0);
                //setImage(convertQtImage(sp->getImage()));
            }
        }
    }

    void clickMousePixel(int x, int y,QMouseEvent * event)
    {
        
        if (event->button() == Qt::RightButton) //usar para tests
        {
            
            //select superpixel
            last_pixel_x = pixel_x;
            last_pixel_y = pixel_y;
            pixel_x= x; pixel_y = y;
        }
        else
        {
            pixel_x= x; pixel_y = y;
            //si blur activado: cambiar color
            if (checkbox->checkState()== Qt::CheckState::Checked)
            {
                //pincel a rojo
                original->setColorBlush(QColor::fromRgb(255, 0,0));
                selection=true;

            }
            else
            {
                AddEq=true;
            }
        
        
            updatePixel();
            update();
            //sp->addEquationsUnaries(sp->getIdFromPixel(x, y), (float) slider->value()/255.0);
        }
        
       /* if (event->button() == Qt::LeftButton)
        {
            printf( " click LEFT %d %d ",x,y) ;fflush(stdout);
           sp->addEquationsUnaries(sp->getIdFromPixel(x, y), 1.0);
        }
        else if(event->button() == Qt::RightButton)
        {
             printf( " click RIGHT %d %d ",x,y) ;fflush(stdout);
           sp->addEquationsUnaries(sp->getIdFromPixel(x, y), 0.0);
        }*/
       
       //  setImage(convertQtImage(sp->getImage()));
        
       // printf( " valor id %d = %d ",sp->getIdFromPixel(x, y),original->near) ;fflush(stdout);
       // sp->addEquationsUnaries(sp->getIdFromPixel(x, y), float(original->near));
        
       // sp->solve();
       // setImage(convertQtImage(sp->getImage()));
        
       // repaint();

      /*  if (event->button() == Qt::LeftButton) // SELECCIONAR
        {
            //update image
            if (sp->isNotNullImage() && sp->isNotNullDepth() && sp->isNotNullIndex())
            {
                if (paintSuperpixel()!=0)
                {
                    last_pixel_x = pixel_x;
                    last_pixel_y = pixel_y;
                    //pixel_x= x; pixel_y = y;
                    setImage(convertQtImage(sp->getImage()));

                    //update label spinner, label y slider with superpixels' depth
                    int depth=sp->getDepthFromPixel(pixel_x,pixel_y)*255;
                    spinner->setValue(depth);
                    selection=true;
                }
                else
                {
                    setInfo("Depth zero, this superpixel should be painted...");
                    last_pixel_x=-1;
                    last_pixel_y=-1;
                    selection=false;

                }
            }
            //printf( " Pulsar IZQ raton (%d,%d)\n ",x,y) ;fflush(stdout);
        }
        else if (event->button() == Qt::RightButton) //COPIAR
        {
             // printf( " Pulsar DERECHO raton (%d,%d)\n ",x,y) ;fflush(stdout);
            if (sp->isNotNullImage() && sp->isNotNullDepth() && sp->isNotNullIndex())
            {
               // if (last_pixel_x!= -1 && last_pixel_y != -1)
               // {
                    updatePixel();
                    repaintSuperpixel();
                    setImage(convertQtImage(sp->getImage()));
                    selection=true;
             //   }
            }
        }*/
    }
    

    void unclickMousePixel(int x, int y,QMouseEvent * event)
    {
        pixel_x= x; pixel_y = y;
        
        if (event->button() == Qt::RightButton) //usar para tests
        {
            
            //select other superpixel
           // sp->resetImage();
            paintSuperpixel();
            
            //compare
        }
        else
        {
            updatePixel();
            if (AddEq)
            {
                propagate();
                AddEq=false;
            }
            
            if (checkbox->checkState()== Qt::CheckState::Checked)
            {
                
                blurImage();
                selection=false;
                
            }
            AddEq=false;
        }

      /*  if (event->button() == Qt::RightButton) // EVENT_LBUTTONDOWN)
        {
           // printf( " Soltar DRCHO raton (%d,%d) LAST (%d,%d)\n ",x,y,last_pixel_x,last_pixel_y) ;fflush(stdout);
            selection=false;
        }
        else if (event->button() == Qt::LeftButton)
        {
            if (sp->isNotNullImage() && sp->isNotNullDepth() && sp->isNotNullIndex())
            {
                if (selection)//last_pixel_x!= -1 && last_pixel_y != -1 )
                {
                    updatePixel();
                    repaintSuperpixel();
                    setImage(convertQtImage(sp->getImage()));
                    selection=false;
                }
            }
           // printf( " Soltar Boton IZQ (%d,%d) LAST (%d,%d)\n ",x,y,last_pixel_x,last_pixel_y) ;fflush(stdout);
        }*/
    }
    
    


    void zoomIn()
    {
         zoom(1.25);
    }

    void zoomOut()
    {
         zoom(0.8);
    }

    void zoomOriginal()
    {
         setZoom();
    }

    void zoomFit()
    {
         zoom(std::min(double(scrollArea->viewport()->width())/double(original->width()),
                          double(scrollArea->viewport()->height())/double(original->height())));
       /* zoom(std::min(double(scrollArea->viewport()->width())/double(imageLabel->width()),
                      double(scrollArea->viewport()->height())/double(imageLabel->height())));
*/
    }

    void stopProgressBar()
    {
        bar->setVisible(false);
        QApplication::setOverrideCursor(Qt::ArrowCursor);
    }
    
    void changeWeightUnaries(int value)
    {
        //sp->w_unary= (float)value/100.0;
        propagate();
    }

    void changeDepth(int value)
    {
        std::stringstream sstr;
        sstr<<"QLabel { background-color :  rgb(" << value << ","<< value <<"," << value <<"); border: 2px solid black; border-radius: 5px}";//rgb(" << 255-value << ","<< 255-value <<"," << 255-value <<");}";
        QString color;
        color = QString(sstr.str().c_str());
        colorSelected->setStyleSheet(color);
        
        original->changeColorClick(value);
    }

    void selectDepth()
    {
         if (sp !=0 ){
             //printf("pulsado"); fflush(stdout);
              // if(  sp->isNotNullImage() && sp->isNotNullDepth() && sp->isNotNullIndex())
             //  {
                  //  sp->copyDepth(x,y,d);
              //     last_pixel_x=-1;
             //      last_pixel_y=-1;
                 //sp->resetImage();
                // sp->depthWithBorders();

             //    setImage(convertQtImage(sp->getImage()));
              // }
         }
    }
    QImage convertQtImage(Mat _image)
    {
        //QImage point to the data of _image
        return QImage(_image.data, _image.cols, _image.rows, _image.step, QImage::Format_RGB888);
    }
    Mat convertMatQtImage(QImage &img, int format)
    {
        return cv::Mat(img.height(), img.width(),
                       format, img.bits(), img.bytesPerLine());
    }
    
    void propagate()
    {
       // sp->solve();
        Mat f=sp->getImage();
        //cv::resize(f, small, Size(640,480));
        setImage(convertQtImage(f));
       // blurImage();
    }
    void updateSizeFocus(int size)
    {
        
        _sizeFocus = double (size)/10.0;
        blurImage();
        
        cout << "Focus: "<< size << "   size: "<< _sizeFocus << endl;
    }
    
    void changeBinaryEquations ( int state )
    {
        if (state==2)//activado => equaciones bianrias de boundaries
        {
            //sp->addEquationsBinariesBoundaries();
        }
        else{
            //sp->addEquationsBinaries();
        }
        propagate();
    }
    void stateChangedCheckBox ( int state )
    {
        
        original->resetOriginal();
        
        if (state==2)
        {
            cout << " check" <<endl;
            buttonEffect->setEnabled(true);
            sliderFocus->setEnabled(true);
            spinner->setDisabled(true);
            slider->setDisabled(true);

        }
        else
        {
            buttonEffect->setDisabled(true);
            spinner->setEnabled(true);
            slider->setEnabled(true);
            sliderFocus->setDisabled(true);
            original->changeColorClick(spinner->value());
        }
    
    }
    
    void blurImage()
    {
        cout<< "Add blur image "<<endl;
       /* Mat image;
        //cvtColor(sp->_lab,image,CV_Lab2BGR);
        Mat imageDepth = sp->_pixelDepth*255.0;
        int nbins=16;
        Mat f=  sp->blurImage(image, imageDepth, nbins,_minFocus,_maxFocus,_sizeFocus);//sp->blurImage(image, imageDepth, nbins);
        cvtColor(f,f,CV_RGB2BGR);
        //setImage(convertQtImage(f));
      //  cv::resize(f, small, Size(640,480));
        setImage(convertQtImage(f));
        
        _minFocus = 255.0;
        _maxFocus = 0.0;
       // imshow("Gaussian Blur",f);
       // waitKey(0);
        
        original->resetOriginal();*/
    }

public:
    ImageWidget() : 
        imageLabel(new QLabel),//ImageLabel),
        scrollArea(new QScrollArea),
    
    
        pixel_x(0), pixel_y(0),
        last_pixel_x(-1),last_pixel_y(-1),
        zoomLabel(new QLabel),
    
    
    
        pixelLabel(new QLabel),
        info(new QLabel),
        original(new ImageLabel),//QLabel),
        colorSelected (new QLabel())
    {
        //Global layout
        QVBoxLayout* layout = new QVBoxLayout;

        //For button bar
        QWidget* buttonBar = new QWidget;
        QHBoxLayout* buttonLayout = new QHBoxLayout;
        buttonBar->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        
        QPushButton* buttonZoomIn  = new QPushButton(QIcon::fromTheme("zoom-in"),"Zoom-in");
        QPushButton* buttonZoomOut = new QPushButton(QIcon::fromTheme("zoom-out"),"Zoom-out");//new QPushButton;
        QPushButton* buttonZoomOriginal = new QPushButton(QIcon::fromTheme("zoom-original"),"Zoom-original");//new QPushButton;
        QPushButton* buttonZoomFit = new QPushButton(QIcon::fromTheme("zoom-fit-best"),"Zoom-fit-best");//new QPushButton;

        buttonLayout->addWidget(buttonZoomIn);
        buttonLayout->addWidget(buttonZoomOut);
        buttonLayout->addWidget(buttonZoomOriginal);
        buttonLayout->addWidget(buttonZoomFit);
        buttonLayout->addStretch();
        
        QObject::connect(buttonZoomIn, &QPushButton::released,
                         this, &ImageWidget::zoomIn);
        QObject::connect(buttonZoomOut, &QPushButton::released,
                         this, &ImageWidget::zoomOut);
        QObject::connect(buttonZoomOriginal, &QPushButton::released,
                         this, &ImageWidget::zoomOriginal);
        QObject::connect(buttonZoomFit, &QPushButton::released,
                         this, &ImageWidget::zoomFit);
        

        //Color spinner and colorSelected
        QLabel * labelcolor  = new QLabel("Depth:");

        spinner = new QSpinBox();
        spinner->setRange(0, 255);

        colorSelected ->setMinimumSize( QSize( 50, 8 ) );
        //add blur
        buttonEffect  = new QPushButton(QIcon::fromTheme("Blur"),"Blur");
        QObject::connect(buttonEffect, &QPushButton::released,
                         this, &ImageWidget::blurImage);
        buttonEffect->setDisabled(true);
        
        checkboxBoundary = new QCheckBox("B&oundary binary", this);
        QObject::connect(checkboxBoundary,  SIGNAL(stateChanged(int)), this, SLOT(changeBinaryEquations(int)));
        
        checkbox = new QCheckBox("A&ctive Blur", this);
        //conectar
        QObject::connect(checkbox,  SIGNAL(stateChanged(int)), this, SLOT(stateChangedCheckBox(int)));
        
        //sliderFocus
        sliderFocus = new QSlider(Qt::Horizontal);
        sliderFocus->setTickPosition(QSlider::NoTicks);
       // sliderFocus->setTickInterval(100);
        sliderFocus->setRange(1,250);
        sliderFocus->setDisabled(true);
        
        QObject::connect(sliderFocus, SIGNAL(valueChanged(int)), this, SLOT(updateSizeFocus(int)));
        
        //peso de ecuaciones unarias frente a bianrias
        spinner_w_u = new QSpinBox();
        spinner_w_u->setRange(1, 100);
        spinner_w_u->setValue(30);
        
        
        QObject::connect(spinner_w_u, SIGNAL(valueChanged(int)), this, SLOT(changeWeightUnaries(int)));
        
        buttonLayout->addWidget(new QLabel("W_U:"));
        buttonLayout->addWidget(spinner_w_u);
        
        
        buttonLayout->addWidget(checkboxBoundary);
        buttonLayout->addWidget(checkbox);
        buttonLayout->addWidget(sliderFocus);
        
        buttonLayout->addWidget(buttonEffect);
        //
        buttonLayout->addWidget(labelcolor);
        buttonLayout->addWidget(spinner);
        buttonLayout->addWidget(colorSelected);

        buttonBar->setLayout(buttonLayout);


        slider = new QSlider(Qt::Horizontal);
        slider->setRange(0,255);
        slider->setTickPosition(QSlider::TicksBelow);
        slider->setTickInterval(1);
        slider->setRange(0,255);

        QObject::connect(spinner, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));

        QObject::connect(slider, SIGNAL(valueChanged(int)), spinner, SLOT(setValue(int)));

        QObject::connect(spinner, SIGNAL(valueChanged(int)), this, SLOT(changeDepth(int)));
        QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(changeDepth(int)));

        QObject::connect(slider, SIGNAL(sliderPressed()), this, SLOT( selectDepth()));
      //  QObject::connect(spinner, SIGNAL(valueChanged(int)), this, SLOT( selectDepth()));
       // QObject::connect(spinner, spinner::QAbstractSpinBox::mousePressEvent(QMouseEvent *event), this, SLOT( selectDepth()));
        

        QVBoxLayout *layoutColor = new QVBoxLayout;

        QLabel *rangeColorDepth= new QLabel();
        //rangeColorDepth->setStyleSheet("QLabel {background-color: qlineargradient(x1:0, y1:0, x1:1, y2:0,stop:0 white,stop:0.5 gray, stop:1 black)}");

        rangeColorDepth->setStyleSheet("QLabel {background-color: qlineargradient(x1:0, y1:0, x1:1, y2:0,stop:0 rgb(0,0,255),stop:0.5 rgb(0,125,255), stop:1 rgb(0,255,255))}");
        
        rangeColorDepth->setContentsMargins(-1, 0, -1, -1);
        layoutColor->setSpacing(0);

        layoutColor->addWidget(slider);
        layoutColor->addWidget(rangeColorDepth);

        QWidget* buttonBarColor = new QWidget;
        buttonBarColor->setLayout(layoutColor);
        buttonBarColor->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

        layout->addWidget(buttonBar);
        layout->addWidget(buttonBarColor);

        //GRID IMAGES
        QSplitter *s1 = new QSplitter();

        QScrollArea* im_original = new QScrollArea;
        //For scroll area imageLabel
        im_original->setBackgroundRole(QPalette::Base);
        im_original->setWidget(original);
        im_original->setAlignment(Qt::AlignCenter);
       
       // original->m_flag=true;
        
        /*im_original->setAlignment(Qt::AlignCenter);
        original = new ImageLabel();//new QLabel();
        im_original->setWidget(original);
        im_original->setWidgetResizable(true);*/
        //  im_original->setAutoResize(true);
       /* QSizePolicy policy = QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        policy.setHeightForWidth(true);*/

        //For scroll area imageLabel
        scrollArea = new QScrollArea;
        scrollArea->setBackgroundRole(QPalette::Base);
        scrollArea->setWidget(imageLabel);
        scrollArea->setAlignment(Qt::AlignCenter);
        
        s1->addWidget(im_original);
        
        s1->addWidget(scrollArea);

        layout->addWidget(s1);

        // For status bar
        QWidget* statusBar = new QWidget;
        QHBoxLayout* statusLayout = new QHBoxLayout;
        statusBar->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

        zoomLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        pixelLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        info->setFrameStyle(QFrame::Panel | QFrame::Sunken);

        bar = new QProgressBar(this);

        statusLayout->addWidget(info);
        statusLayout->addWidget(bar);
        statusLayout->addStretch();
        statusLayout->addWidget(zoomLabel);
        statusLayout->addWidget(pixelLabel);
        layout->addWidget(statusBar);

        statusBar->setLayout(statusLayout);

        updatePixel(); updateZoom();

        QObject::connect(original, &ImageLabel::mousePixelChanged,
                         this, &ImageWidget::setMousePixel);
        //
        /*QObject::connect(original, &ImageLabel::mousePixelDown,
                         this,&ImageWidget::paintEvent);*/
       
        //
        
        QObject::connect(original, &ImageLabel::mousePixelDown,
                         this, &ImageWidget::clickMousePixel);
        QObject::connect(original, &ImageLabel::mousePixelUp,
                         this, &ImageWidget::unclickMousePixel);

        setLayout(layout);

        //selection pixel
        selection=false;
    }
 
    ~ImageWidget() { }
};



#endif
