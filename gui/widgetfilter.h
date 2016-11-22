#ifndef WIDGETIMAGE_H
#define WIDGETIMAGE_H

#include <QWidget>
#include <QScrollArea>
#include <QApplication>
#include <QLabel>
#include <QSlider>
#include "labelimage.h"
#include "imagemousebrush.h"
#include "multiimageviewer.h"
#include "../filters/filter.h"

#include "denseLabeling.cpp"
#include "imageHDR.h"


class WidgetFilter : public QWidget
{
    Q_OBJECT
    
private:
	    const Filter& filter;

	    std::shared_ptr<QPixmap> strokes_pixmap;
	    std::shared_ptr<cv::Mat> input_image;
	    std::shared_ptr<cv::Mat> filtered_image;
	    std::vector<std::shared_ptr<cv::Mat>> propagated_channels;


    	    ImageMouseBrush *imageMouseBrush;
            MultiImageViewer *multiImageViewer;
    	    QButtonGroup *buttonOptions;
	    QVBoxLayout  *buttonLayout;

    	    int buttonIdStrokes, buttonIdInput, buttonIdFiltered;
    
            //Datos
	    std::vector<DenseLabeling*> labels;
    
            //valor de profundidad
            double _minFocus = 220, _maxFocus=0.0;
            double _sizeFocus = 50.0;

            float brush = 255.0;
	    DenseLabeling* activeLabeling = NULL;
	    std::vector<float> brushValues;
	    std::vector<int> brushChannels;
   	    int button_id; 

	    std::vector<QSlider*> sliderValues;

    void chooseButton(int id)
    {
	    int v = 255/(id/3 + 1);
	    imageMouseBrush->setColorBrush(QColor::fromRgb(v*((id)%3), v*((id+1)%3), v*((id+2)%3));
            this->setCursor(Qt::PointingHandCursor);
	    brush = brushValues[id];
	    activeLabeling = labels[id];
    }

    void updateSlider(int size)
    {
        processImage();
    }

public:

    	int addBrush(const char* name, float value, int channel=0)
    	{
		QPushButton* button = new QPushButton(QString(label));
		button->setCheckable(true);
		buttonLayout.addWidget(button);
		buttonOptions.addButton(button, button_id);
		brushValues.push_back(value);
		brushChannels.push_back(channel);
		if (button_id == 0) {
			button->setChecked(true);
		}
		return button_id++;
	}
    
    //construir la interfaz
    WidgetFilter(const Filter& f) : filter(f),
	    strokes_pixmap(std::make_shared<QPixmap>()), input_image(std::make_shared<cv::Mat>()), 
	    filtered_image(std::make_shared<cv::Mat>()), propagated_channels(filter.propagatedValues().size()),
	    imageMouseBrush(new ImageMouseBrush(input_image, strokes_pixmap)), multiImageViewer(new MultiImageViewer()),
	    labels(filter.propagatedValues().size()),
	    buttonOptions(new QButtonGroup()), buttonLayout(new ButtonsLayout()),
	    button_id(0);
    {
	    for (std::shared_ptr<cv::Mat>& channel : propagated_channels) channel = std::make_shared<cv::Mat>();
        QHBoxLayout *layoutH = new QHBoxLayout;
        this->setMinimumSize(400,400);
        
        QVBoxLayout *sideBar = new QVBoxLayout;
        
        QGroupBox *boxOptions;
        boxOptions = new QGroupBox("Select options:");
        boxOptions->setMinimumSize(30,320);//(180,320);
       
        QSlider* sliderFocus;
        sliderFocus = new QSlider(Qt::Horizontal);
        
        sideBar->addLayout(buttonsLayout);
        sideBar->addStretch();
	for (auto v : filter.floatValues()) {
		float min, max; std::string name;
		std::tie(name,min,max) = v;
		QSlider* sliderValue = new QSlider(Qt::Horizontal);
	        sliderValue->setTickPosition(QSlider::TicksAbove);
        	sliderValue->setTickInterval(10);
        	sliderValue->setRange(1,100);
        	sliderValue->setValue(50);
        	sliderValue->setEnabled(true);
		sliderValue->setTracking(false);
//      	sliderFocus->setFixedSize(180,20);
		sliderValues.push_back(sliderValue);
        	sideBar->addWidget(sliderValue);
	}

        
        layoutH->addLayout(sideBar);
        
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

	for (int i=0;i<propagated_channels.size();++i) {
		multiImageViewer->add(filter.propagatedValues()[i],propagated_channels[i]);
	}
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
    
    
   
    void resetOptions()
    {
        this->setCursor(Qt::ArrowCursor);
        _minFocus = 250.0;
        _maxFocus = 255.0;
        destroyAllWindows();
        //buttonOptions->resetSelection();
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
//

	    for (DenseLabeling*& d : labels) { 
		    d = new DenseLabeling(filename.toStdString(),0.3,0.99,10.0);
	            d->addEquations_BinariesBoundariesPerPixelMean();
	    }
        
//        if (!denseDepth->isNotNullImage())
//        {
//            setInfo("Problem to open the image!");
//            return false;
//        }
//        else
//        {
//            setInfo("Image opened correctly.");
//        }
        //binary equations
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
	    activeLabeling->addEquation_Unary(x,y,value/255.0);
	    multiImageViewer->setButton(buttonIdStrokes);
        }
    }
    
    void clickMousePixel(int x, int y,QMouseEvent * event)
    {
        if (event->button() == Qt::LeftButton)
        {
            if (input_image)
            {
		    setInfo("Add new unary equation");
		    activeLabeling->addEquation_Unary(x,y,value/255.0);
		    multiImageViewer->setButton(buttonIdStrokes);

                //imageToEdit->enablePaint();
                //si blur activado: cambiar color
               // if (buttonOptions->focusSelected())
                //{
                  //  updateFocus(x,y);
                //}
                //else
                //{
                  //  setInfo("Add new unary equation");
                  //  activeLabeling->addEquation_Unary(x,y,value/255.0);
                //}
            }
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
	for (int i = 0; i<labels.size();++i)
	{
		*(propagated_channels[i])=labels[i]->solve();
	}

	std::vector<float> floatValues(sliderValues.size());
	for (int i = 0; i<sliderValues.size(); ++i)
	{
		float min, max; std::string name;
		std::tie(name,min,max) = filter.floatValues()[i];
		floatValues[i]=float(sliderValues[i]->value())*(max-min) + min;
	}
        
        *filtered_image = filter.apply(*input_image, propagated_channels, floatValues);

/*	
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
*/
	
	multiImageViewer->setButton(buttonIdFiltered);
//        if (buttonOptions->idSelected() != -1)
//            imageToEdit->set(filtered_image);//*/
    }
};

#endif
