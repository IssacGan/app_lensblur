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

    const int slider_ticks = 1000;

class Brush {
	
public:
	virtual void onClicked(int x, int y) { };
	virtual void onMoved(int x, int y)   { };
	virtual bool shouldDrawStroke() const { return false; }
};

class BrushEditChannel : public Brush
{
	const std::vector<DenseLabeling*>& labels;
	std::vector<bool>& edited;
	std::list<std::tuple<float, int>> values;
public:
	BrushEditChannel(const std::vector<DenseLabeling*>& _labels, 
			std::vector<bool>& _edited, const std::list<std::tuple<float, int>> _values) :
	       labels(_labels), edited(_edited), values(_values) { }
	BrushEditChannel(const std::vector<DenseLabeling*>& _labels, 
			std::vector<bool>& _edited, float _value, float _channel) : 
		BrushEditChannel(_labels, _edited, std::list<std::tuple<float,int>>{{std::make_tuple(_value, _channel)}}) { }

	void onClicked(int x, int y) override {
		for (auto t : values) {
			float value; int channel;
			std::tie(value, channel) = t;
	    		if (!edited[channel]) { //We update the edited thing only on clicked (a bit extra efficiency)
	    			edited[channel] = true;
				labels[channel]->clearUnaries(); //We remove the initial equation that sets up the entire thing.
	    		}
		}
	}

	void onMoved(int x, int y) override { //This event also happens when clicking.
		for (auto t : values) {
			float value; int channel;
			std::tie(value, channel) = t;
	    		labels[channel]->addEquation_Unary(x,y,value);
		}
	}

	bool shouldDrawStroke() const override { return true; }
};

class BrushPickValue : public Brush
{
	std::shared_ptr<cv::Mat> from;
	QSlider* slider;
	float min, max;
public:
	BrushPickValue(const std::shared_ptr<cv::Mat>& _from, QSlider* _slider, float _min, float _max) :
		from(_from), slider(_slider), min(_min), max(_max) { }

	void onMoved(int x, int y) override { //This event also happens when clicking.
		if (from && slider && (x>=0) && (x<from->cols) && (y>=0) && (y<from->rows))
		{
			float v = from->at<float>(y,x,0);
			slider->setValue(int(float(slider_ticks)*(v-min)/(max-min)));
		}
	}
};

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
   
   	    int button_id; 

	    std::vector<QSlider*> sliderValues;

	    std::vector<std::shared_ptr<Brush>> brushes;
	    std::shared_ptr<Brush> activeBrush;

    void chooseButton(int id)
    {
	    if ((id>=0) && (id<brushes.size())) {
            	    this->setCursor(Qt::PointingHandCursor); //Maybe have a look at the cursors somewhen
		    activeBrush=brushes[id];
		    if (activeBrush->shouldDrawStroke())
	    		    imageMouseBrush->setColorBrush(QColor::fromHsv((100*id)%360, 255, 255));
		    else
			    imageMouseBrush->unsetBrush();
	    }
    }

    void updateSlider(int size)
    {
	if ((input_image) && (!input_image->empty())) processImage();
    }

    std::vector<bool> edited;

public:

    	int addBrush(const char* name, float value, int channel=0)
    	{
		QPushButton* button = new QPushButton(QString(name));
	        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		button->setCheckable(true);
		buttonLayout->addWidget(button);
		buttonOptions->addButton(button, button_id);
		brushes.push_back(std::make_shared<BrushEditChannel>(labels, edited, value, channel));
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
       
        sideBar->addWidget(brushBox);

        QVBoxLayout *sliderLayout = new QVBoxLayout();
        QGroupBox *sliderBox = new QGroupBox(tr("Parameters"));
	sliderBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	for (auto fv : filter.floatValues()) {
		QLabel* labelValue = new QLabel(fv.name().c_str());
		sliderLayout->addWidget(labelValue);
		QSlider* sliderValue = new QSlider(Qt::Horizontal);
	        sliderValue->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	        sliderValue->setTickPosition(QSlider::TicksAbove);
        	sliderValue->setTickInterval(slider_ticks/10);
        	sliderValue->setRange(0,slider_ticks);
        	sliderValue->setValue(slider_ticks/2);
        	sliderValue->setEnabled(true);
		sliderValue->setTracking(false);
//      	sliderFocus->setFixedSize(180,20);
		sliderValues.push_back(sliderValue);
        	sliderLayout->addWidget(sliderValue);
		QObject::connect(sliderValue,&QSlider::valueChanged,this,&WidgetFilter::updateSlider);

	        if (fv.isPickable()) {
			QPushButton* button = new QPushButton(QString("^ Pick ^"));
	        	button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			button->setCheckable(true);
			sliderLayout->addWidget(button);
			buttonOptions->addButton(button, button_id++);
			brushes.push_back(std::make_shared<BrushPickValue>(propagated_channels[fv.channelToPickFrom()],sliderValue, fv.min(), fv.max()));
		}	
	}
	sliderBox->setLayout(sliderLayout);
	sideBar->addWidget(sliderBox);
        sideBar->addStretch();

        
        layoutH->addLayout(sideBar);
	layoutH->addWidget(multiImageViewer);
        
        QVBoxLayout *image = new QVBoxLayout;
        image->addWidget(multiImageViewer);
       
        //BUTTONS FOR STORKES	
	for (auto stroke : filter.strokes()) {
		QPushButton* button = new QPushButton(QString(stroke.name().c_str()));
	        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
		button->setCheckable(true);
		buttonLayout->addWidget(button);
		buttonOptions->addButton(button, button_id);
		brushes.push_back(std::make_shared<BrushEditChannel>(labels, edited, stroke.values()));
		if (button_id == 0) {
			button->setChecked(true);
			chooseButton(0);
		}
		button_id++;
	}
        
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
		multiImageViewer->add(filter.propagatedValues()[i].name().c_str(),propagated_channels[i]);
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
			float min = filter.floatValues()[i].min();
			float max = filter.floatValues()[i].max(); 
			floatValues[i]=(float(sliderValues[i]->value())/float(slider_ticks))*(max-min) + min;
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
	
	    for (int i = 0; i < labels.size(); ++i) { 
		    if (labels[i]) delete labels[i];
		    labels[i] = new DenseLabeling(filename.toStdString(),0.3,0.99,10.0);
	            labels[i]->addEquations_BinariesBoundariesPerPixelMean();
		    labels[i]->addEquation_Unary(0,0,filter.propagatedValues()[i].defaultValue());
	    }
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
		activeBrush->onMoved(x,y);
		if (activeBrush->shouldDrawStroke()) 
	    		multiImageViewer->setButton(buttonIdStrokes);
        }
    }
    
    void clickMousePixel(int x, int y,QMouseEvent * event)
    {
        if (event->button() == Qt::LeftButton)
        {
            if (input_image)
            {
		    activeBrush->onClicked(x,y);
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

    
    void processImage(bool save=false, string dir = "")
    {
	std::stringstream sstr;
    	std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
	solveAll();
	std::chrono::duration<double> elapsed_seconds = std::chrono::system_clock::now() - start;
	sstr<<"Propagation: "<<std::setprecision(6)<<std::setw(8)<<std::fixed<<elapsed_seconds.count()<<" seconds";
    	start = std::chrono::system_clock::now();
	*filtered_image = filter.apply(*input_image, propagated_channels, filterParameters());
	elapsed_seconds = std::chrono::system_clock::now() - start;
	sstr<<"        Image processing: "<<std::setprecision(6)<<std::setw(8)<<std::fixed<<elapsed_seconds.count()<<" seconds";
	setInfo(sstr.str().c_str());
	multiImageViewer->setButton(buttonIdFiltered);
    }
};

