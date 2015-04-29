#ifndef SUPERPIXELS_H
#define SUPERPIXELS_H


//Opencv
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
//Image Qt
//#include <QImage>
using namespace cv;

#include "superpixel.h"

//extern "C" {
    #include "vl/generic.h"
    #include "vl/slic.h"
//}

//sistema de ecuaciones
#include <math.h>
#include <vector>
#include <iomanip>
using namespace std;
#include "least-squares-linear-system2.h"
using namespace Eigen;
typedef Matrix<double, Dynamic, 1> MatrixFX;



#define RED 0
#define GREEN 1
#define RANDOM 2

#define DEPTH_NEAR 224.0
#define DEPTH_FAR_1 160.0
#define DEPTH_FAR_2 96.0
#define DEPTH_FAR_3 32.0

class SuperPixels
{
public:
    Mat _image;
    
    Mat _depth;
    Mat _ids;
    Mat _imDepth;
    Mat _pixelDepth;
    
    Mat _lab;
    
    int showInfo;
    double w_unary=0.8;
    double w_color=0.9;
    
    double _MAX_DIFF_LAB = 15.0;
    
    int _NUM_MAX_SP = 600;
    int _TAM_SP = 35;//35;//25;
    
    ////// Superpixels funciones
    int numCoef=3;
    
    Optimization::LeastSquaresLinearSystem<double> ecuaciones;
    Optimization::LeastSquaresLinearSystem<double> unary;
    Optimization::LeastSquaresLinearSystem<double> unaryCTE;
    
    Optimization::LeastSquaresLinearSystem<double> binary;
    Optimization::LeastSquaresLinearSystem<double> binaryCOLOR;
    Optimization::LeastSquaresLinearSystem<double> binaryGRADIENT;//
    
//public:
    SuperPixel *arraySP;
    Mat _sobel;
   // Mat _pixelDepth;
    int maxID;
    SuperPixels(std::string path);
    ~SuperPixels() ;

    //QImage point to the data of _image
   // QImage getImage();
    Mat getImage();
    Mat imageDepth();
    Mat getMask(int id);
    void updateDepth(Mat depth);
    
    //QImage mat_to_qimage_ref(Mat &mat, QImage::Format format);
    void loadSuperPixels(std::string path);
    void calculateSLICSuperpixels(Mat mat);
    void loadDepth(std::string path);
    void loadUserDepth(std::string path);
    void loadDepthBuildSystem(std::string path);
    void loadDepthBuildSystemCoef();
    void loadDepthBuildSystemCoef(std::string path);

    Mat paintSuperpixel(int i,int y);
    Mat paintSuperpixel(int i,int y,Scalar *color);
    void paintSuperpixelByID(int id,int grayLevel);
    void copyDepth(int x, int y, float depth);
    void copySuperpixel(int x,int y, int last_x,int last_y);
    
    void calcularVecinos();
   // void calcularVecinos2();
    void calcularVecinos2(int id, int *array);

    int getIdFromPixel(int x, int y);
    float getDepthFromPixel(int x, int y);
    float getDepthInitialFromPixel(int x, int y);
    bool isNotNullImage();
    bool isNotNullDepth();
    bool isNotNullIndex();

    void resetImage();
    void paintZeroDepth();
    void depthWithBorders();
    
    void infoSuperpixels2file(std::string nameFile);

    float medianHistogram(MatND hist,int numPixels);
    
  //  MatND calHistogram(Mat m, Mat mask);
   // void histogramLAB(int id,Mat mask);
    float cmpDepth(int a, int b, int mode);
    float cmpLab(int a, int b, int mode);
    float cmpLab(int a, SuperPixel b, int mode);
    
    float getDepth(int i);
    float getVar(int i);
    float getAccu(int i);
    
    //equations
    void addEquationsBinaries();
    void addEquationsBinariesCoef();
    void addEquationsBinariesBoundaries(bool verbose);
    void addEquationsBinariesBoundariesCoef(bool verbose=false);
    void addEquationsBinariesGradientCoef();
    float similarityBinariesBoundaries(int s1, int s2);
    void addEquationsUnaries(int id,float di);
    void addEquationsUnaries(int x,int y,float di);
    void addEquationsUnaries();
    void addUnariesCoef(int x, int y, double di);
    void addEquationsUnariesCoef();
    void solve();
    void solveCoef();
    
    
    //effects
    Mat blurImage(Mat image, Mat imageDepth, int nbins);
    Mat blurImage(Mat image, Mat imageDepth, int nbins, double minFocus, double maxFocus,double size);
    //code adolfo
    Mat blurImageDepth(const cv::Mat& image, const cv::Mat& depth,
                         int nbins, float focal_distance, float focal_length, float aperture, bool linear);

};

#endif // SUPERPIXELS_H
