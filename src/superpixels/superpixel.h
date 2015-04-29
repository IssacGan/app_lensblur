#ifndef SUPERPIXEL_H
#define SUPERPIXEL_H


//Opencv
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;


class SuperPixel
{
public:
    int id;
    //mascara de la imagen original
    Mat mask;
    int numPixels;
    
    MatND hist_l;
    float l_median;
    float l_mean;
    float l_std;
    
    MatND hist_a;
    float a_median;
    float a_mean;
    float a_std;
    
    MatND hist_b;
    float b_median;
    float b_mean;
    float b_std;
    
    
    
    MatND hist_depth;
    //mediana sin ceros
    float d_median;
    //media
    float d_mean;
    //varianza
    float d_var;
    //precision
    float d_acc;
    float depth;

//public:
    
    SuperPixel();
    ~SuperPixel() ;
    
    void init(int id,Mat mask,int numPixels);
    
    float medianHistogram(MatND hist,int num);
    
    float meanHistogram(MatND hist,int num);
    float varHistogram(MatND hist,float mean,int num);
    float medianDepth(Mat _depth);//,MatND h);//, float accurancy);
    void meanVarDepth(Mat depth);
    
    void descriptorsLab(Mat image);
   
    
    
    void showHistogram(MatND h);
    void showDepth();

    float cmpHistogram(MatND a,MatND b,int mode);
    
    
    //TODO
    //calcular las vecinas de un supepixels calcularVecinas2()
    //guardar solo los ids mayores
    
    
};

#endif // SUPERPIXEL_H
