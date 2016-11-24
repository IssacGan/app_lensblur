#pragma once 

#include <QWidget>
#include <QScrollArea>
#include <QApplication>
#include <QLabel>
#include <QSlider>
#include <QGroupBox>
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
            QLabel* info;

    	    int buttonIdStrokes, buttonIdInput, buttonIdFiltered;
    
            //Datos
	    std::vector<DenseLabeling*> labels;
    
            //valor de profundidad
            double _minFocus = 220, _maxFocus=0.0;
            double _sizeFocus = 50.0;

            float brush = 255.0;
	    float brushChannel = 0;
	    DenseLabeling* activeLabeling = NULL;
	    std::vector<float> brushValues;
	    std::vector<int> brushChannels;
   	    int button_id; 

	    std::vector<QSlider*> sliderValues;

    void chooseButton(int id)
    {
	    imageMouseBrush->setColorBrush(QColor::fromHsv((100*id)%360, 255, 255));
            this->setCursor(Qt::PointingHandCursor);
	    brush = brushValues[id];
	    brushChannel = brushChannels[id];
	    activeLabeling = labels[brushChannel];
    }

    void updateSlider(int size)
    {
        processImage();
    }

    std::vector<bool> edited;

public:

    	int addBrush(const char* name, float value, int channel=0)
    	{
		QPushButton* button = new QPushButton(QString(name));
	        button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		button->setCheckable(true);
		buttonLayout->addWidget(button);
		buttonOptions->addButton(button, button_id);
		brushValues.push_back(value);
		brushChannels.push_back(channel);
		if (button_id == 0) {
			button->setChecked(true);
			chooseButton(0);
		}
		return button_id++;
	}
    
    //construir la interfaz
    WidgetFilter(const Filter& f) : filter(f),
	    strokes_pixmap(std::make_shared<QPixmap>()), input_image(std::make_shared<cv::Mat>()), 
	    filtered_image(std::make_shared<cv::Mat>()), propagated_channels(filter.propagatedValues().size()),
	    imageMouseBrush(new ImageMouseBrush(input_image, strokes_pixmap)), multiImageViewer(new MultiImageViewer()),
	    buttonOptions(new QButtonGroup()), buttonLayout(new QVBoxLayout()), info(new QLabel()),
	    labels(filter.propagatedValues().size()),
	    button_id(0), edited(filter.propagatedValues().size(),false)
    {
	    for (std::shared_ptr<cv::Mat>& channel : propagated_channels) channel = std::make_shared<cv::Mat>();
	    QVBoxLayout *mainLayout = new QVBoxLayout();
            QHBoxLayout *layoutH = new QHBoxLayout();
	    mainLayout->addLayout(layoutH);
            this->setMinimumSize(400,400);
        
            QVBoxLayout *sideBar = new QVBoxLayout();
            QGroupBox *brushBox = new QGroupBox(tr("Brushes"));
	    brushBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	    brushBox->setLayout(buttonLayout);
       
        QSlider* sliderFocus;
        sliderFocus = new QSlider(Qt::Horizontal);
        
        sideBar->addWidget(brushBox);


        QVBoxLayout *sliderLayout = new QVBoxLayout();
        QGroupBox *sliderBox = new QGroupBox(tr("Parameters"));
	sliderBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	for (auto v : filter.floatValues()) {
		float min, max; std::string name;
		std::tie(name,min,max) = v;
		QLabel* labelValue = new QLabel(name.c_str());
		sliderLayout->addWidget(labelValue);
		QSlider* sliderValue = new QSlider(Qt::Horizontal);
	        sliderValue->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	        sliderValue->setTickPosition(QSlider::TicksAbove);
        	sliderValue->setTickInterval(10);
        	sliderValue->setRange(0,100);
        	sliderValue->setValue(50);
        	sliderValue->setEnabled(true);
		sliderValue->setTracking(false);
//      	sliderFocus->setFixedSize(180,20);
		sliderValues.push_back(sliderValue);
        	sliderLayout->addWidget(sliderValue);
		QObject::connect(sliderValue,&QSlider::valueChanged,this,&WidgetFilter::updateSlider);  
	}
	sliderBox->setLayout(sliderLayout);
	sideBar->addWidget(sliderBox);
        sideBar->addStretch();

        
        layoutH->addLayout(sideBar);
	layoutH->addWidget(multiImageViewer);
        
        QVBoxLayout *image = new QVBoxLayout;
        image->addWidget(multiImageViewer);
        
        
        //add label de info
        info->setFrameStyle(QFrame::Panel | QFrame::Sunken);
        info->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        mainLayout->addWidget(info);
        
        setLayout(mainLayout);

	imageMouseBrush->connectToLabelImage(multiImageViewer->labelImage());
	buttonIdInput    = multiImageViewer->add("Input",input_image);
	buttonIdStrokes  = multiImageViewer->add("Strokes",imageMouseBrush->getPixmap());
	buttonIdFiltered = multiImageViewer->add("Filtered",filtered_image);

	for (int i=0;i<propagated_channels.size();++i) {
		multiImageViewer->add(filter.propagatedValues()[i].c_str(),propagated_channels[i]);
	}

        //conectar señales
        //click sobre opcion, cambiar color pincel
	QObject::connect(buttonOptions, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
				this, &WidgetFilter::chooseButton);

        QObject::connect(multiImageViewer->labelImage(), &LabelImage::mousePixelDown,
                         this, &WidgetFilter::clickMousePixel);
        QObject::connect(multiImageViewer->labelImage(), &LabelImage::mousePixelUp,
                         this, &WidgetFilter::unclickMousePixel);
        QObject::connect(multiImageViewer->labelImage(), &LabelImage::mousePixelChanged,
                         this, &WidgetFilter::updatePixel);
        
        resize(sizeHint());
    }
    
    void setInfo(QString sms)
    {
        info->setText(sms);
        QApplication::processEvents();
    }
    
    void resizeEvent(QResizeEvent* event)
    {
	    if ((input_image) && (!input_image->empty())) processImage();
    }
   
    void resetOptions()
    {
        this->setCursor(Qt::ArrowCursor);
        _minFocus = 250.0;
        _maxFocus = 255.0;
        destroyAllWindows();
        //buttonOptions->resetSelection();
    }
    
private:
    void solveAll() {
	for (int i = 0; i<labels.size();++i)
	{
		*(propagated_channels[i])=labels[i]->solve();
	}
    }

    std::vector<float> filterParameters() const { 
		std::vector<float> floatValues(sliderValues.size());
		for (int i = 0; i<sliderValues.size(); ++i)
		{
			float min, max; std::string name;
			std::tie(name,min,max) = filter.floatValues()[i];
			floatValues[i]=(float(sliderValues[i]->value())*(max-min) + min)/100.0f;
		}
		return floatValues;
    }

  
public: 
    //////////////// SUPERPIXELS + BINARY EQUATIONS
    bool loadData(const QString& filename)
    {
	    load_image(filename.toStdString().c_str(), input_image);
	    imageMouseBrush->clear();
	    for (int i = 0; i < edited.size(); ++i) edited[i] = false;
            //mostrar imagen en la interfaz
	    multiImageViewer->setButton(buttonIdInput);
//            imageToEdit->set(input_image);
//            imageMouseBrush->setImage(input_image);
//

	    for (DenseLabeling*& d : labels) { 
		    d = new DenseLabeling(filename.toStdString(),0.3,0.99,10.0);
	            d->addEquations_BinariesBoundariesPerPixelMean();
		    d->addEquation_Unary(0,0,0.5f);
	    }
	    activeLabeling = labels[0];
	    solveAll();
	    *filtered_image = filter.apply(*input_image, propagated_channels, filterParameters());
 
        setInfo("Binary equations created.");
        
        return true;
    }

    //////////////// INPUT UNARY EQUATIONS
    void saveData(QString dir)
    {
        processImage(true,dir.toStdString());
        setInfo("All files Saved.");
    }
    

private:
    ////////////////
    
    //////// MOUSE
    void updatePixel(int x, int y)
    {
        if (input_image)
        {
            setInfo("Add new unary equation");
	    activeLabeling->addEquation_Unary(x,y,brush/255.0);
	    multiImageViewer->setButton(buttonIdStrokes);
        }
    }
    
    void clickMousePixel(int x, int y,QMouseEvent * event)
    {
        if (event->button() == Qt::LeftButton)
        {
            if (input_image)
            {
		    if (!edited[brushChannel]) {
		    	edited[brushChannel] = true;
			activeLabeling->clearUnaries(); //We remove the initial equation that sets up the entire thing.
	            }
			
		    setInfo("Add new unary equation");
		    activeLabeling->addEquation_Unary(x,y,brush/255.0);
		    multiImageViewer->setButton(buttonIdStrokes);
            }
        }
	else if (event->button() == Qt::RightButton)
	{
	    if (input_image) {
		    imageMouseBrush->clear();
		    for (DenseLabeling* d : labels) { 
			    d->clearUnaries();
			    d->addEquation_Unary(0,0,0.5f);
		    }
		    for (int i = 0; i < edited.size(); ++i) edited[i] = false;
		    multiImageViewer->setButton(buttonIdStrokes);
	    }
	}
    }
    
    void unclickMousePixel(int x, int y,QMouseEvent * event)
    {
        processImage();
    }



/*    
    void updateFocus(int x, int y)
    {
        double newDepth = denseDepth->getLabel(x,y) * 255.0;
        
        //if (newDepth < _minFocus)
            _minFocus = newDepth;
       // if (newDepth > _maxFocus)
         //   _maxFocus = newDepth;
        
         //fprintf(stderr,"_min: %f max: %f new: %f",_minFocus,_maxFocus,newDepth);
    }
*/

    
    void processImage(bool save=false, string dir = "")
    {
	solveAll();
	*filtered_image = filter.apply(*input_image, propagated_channels, filterParameters());
	multiImageViewer->setButton(buttonIdFiltered);
    }
};

