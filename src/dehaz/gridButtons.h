#ifndef GRIDBUTTONS_H
#define GRIDBUTTONS_H

#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>

#include <QGridLayout>

#define ID_1 1

#define ID_2 2
#define ID_3 3
#define ID_4 4

#define ID_FOCUS 5


class GridButtons : public QWidget
{
    Q_OBJECT
private:
    
    QButtonGroup *options;

    QRadioButton *portrait;//= new QRadioButton("Cerca" );
    QRadioButton *landscape1; // = new QRadioButton( "Lejos" );
    QRadioButton *landscape2; // = new QRadioButton( "Lejos" );
    QRadioButton *landscape3; // = new QRadioButton( "Lejos" );
    QRadioButton *focus;// = new QRadioButton( "Focus");

    int _id = -1;

   // int icon_size = 60;
    
signals:
    void optionSelected(int _id);

public:

   // constructor del grid de botones
    GridButtons( QWidget *parent = 0 )
    {
        //crear grupo de botones
        QString infoPortrait= "Draw blue strokes with the mouse.\n Select the closest objects.";
        QString infoLandscape= "Draw green strokes with the mouse.\n Select the furthest objects.";
        QString infoFocus= "Draw red strokes.\n Select objects you want to show focused.";

        portrait = new QRadioButton("NO FOG" );
        portrait->setToolTip(infoPortrait);
        
        landscape1 = new QRadioButton( "MIN FOG" );
        landscape1->setToolTip(infoLandscape);
        landscape2 = new QRadioButton( "MEDIUM FOG " );
        landscape2->setToolTip(infoLandscape);
        landscape3 = new QRadioButton( "MAX FOG" );
        landscape3->setToolTip(infoLandscape);
        
        /*focus = new QRadioButton( "Focus");
        focus->setToolTip(infoFocus);*/

        options =  new QButtonGroup( parent );

        options->addButton( portrait );
        
        options->addButton( landscape1 );
        options->addButton( landscape2 );
        options->addButton( landscape3 );
        
       // options->addButton( focus );

        //crear iconos
      /*  QImage *icon1 = new QImage("/Users/acambra/Desktop/camera63.png");
        QImage *icon2 = new QImage("/Users/acambra/Desktop/camera67.png");
        QImage *icon3 = new QImage("/Users/acambra/Desktop/diaphragm8.png");*/

       /* QImage *scaled_icon1 = new QImage(icon1->scaled(icon_size,icon_size,Qt::KeepAspectRatio));
        
        QImage *scaled_icon2_1 = new QImage(icon2->scaled(0.33*icon_size,0.33*icon_size,Qt::KeepAspectRatio));
        QImage *scaled_icon2_2 = new QImage(icon2->scaled(0.66*icon_size,0.66*icon_size,Qt::KeepAspectRatio));
        QImage *scaled_icon2_3 = new QImage(icon2->scaled(icon_size,icon_size,Qt::KeepAspectRatio));
        
        QImage *scaled_icon3 = new QImage(icon3->scaled(icon_size,icon_size,Qt::KeepAspectRatio));*/

      /*  QLabel *label1 = new QLabel;
        QLabel *label2_1 = new QLabel;
        QLabel *label2_2 = new QLabel;
         QLabel *label2_3 = new QLabel;
        QLabel *label3 = new QLabel;

        //label1->setPixmap(QPixmap::fromImage(*scaled_icon1));
        label1->setToolTip(infoPortrait);
        
        label2_1->setPixmap(QPixmap::fromImage(*scaled_icon2_1));
        label2_1->setToolTip(infoLandscape);
        label2_1->setAlignment(Qt::AlignCenter);
       // label2_2->setPixmap(QPixmap::fromImage(*scaled_icon2_2));
        label2_2->setAlignment(Qt::AlignCenter);
        label2_2->setToolTip(infoLandscape);
        label2_3->setPixmap(QPixmap::fromImage(*scaled_icon2_3));
        label2_3->setAlignment(Qt::AlignCenter);
        label2_3->setToolTip(infoLandscape);
        
        label3->setPixmap(QPixmap::fromImage(*scaled_icon3));
        label3->setToolTip(infoFocus);*/

        //crear grid y añadir elementos
        QGridLayout *grid = new QGridLayout( parent );
        grid->addWidget( portrait, 0, 1 );
        grid->addWidget( landscape1, 1, 1 );
        grid->addWidget( landscape2, 2, 1 );
        grid->addWidget( landscape3, 3, 1 );
        //grid->addWidget( focus, 4, 1 );

      /*  grid->addWidget(label1, 0, 0 );
        grid->addWidget(label2_1, 1, 0 );
        grid->addWidget(label2_2, 2, 0 );
        grid->addWidget(label2_3, 3, 0 );
        grid->addWidget(label3, 4, 0 );*/

        //conectar señal
        options-> connect(options, SIGNAL(buttonClicked(int)),this , SLOT(onGroupButtonClicked(int)));

    }
    ~GridButtons() {}



public slots:
    
    bool focusSelected()
    {
        if (_id == ID_FOCUS)
            return true;
        
        return false;
    }

   /* void resetSelection()
    {
        options->setExclusive(false);
        options->checkedButton()->setChecked(false);
        options->setExclusive(true);
    }*/

    void onGroupButtonClicked(int id)
    {
           if (id == options->id(this->portrait))
           {
               fprintf(stderr," button CLICKED Portrait\n");
               _id = ID_1;
               emit optionSelected(_id);
           }
           else if (id == options->id(this->landscape1))
           {
               fprintf(stderr," button CLICKED landscape\n");
               _id = ID_2;
               emit optionSelected(_id);
           }
           else if (id == options->id(this->landscape2))
           {
               fprintf(stderr," button CLICKED landscape\n");
               _id = ID_3;
               emit optionSelected(_id);
           }
           else if (id == options->id(this->landscape3))
           {
               fprintf(stderr," button CLICKED landscape\n");
               _id = ID_4;
               emit optionSelected(_id);
           }
          /* else if (id == options->id(this->focus))
           {
               fprintf(stderr," button CLICKED Focus\n");
               _id = ID_FOCUS;
               emit optionSelected(_id);
           }*/
    }
};

#endif // GRIDBUTTONS_H
