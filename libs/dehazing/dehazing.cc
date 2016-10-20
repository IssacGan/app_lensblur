//
//  main.cpp
//  test_superpixels
//
//  Created by Ana Cambra on 26/11/14.
//  Copyright (c) 2014 Ana Cambra. All rights reserved.
//

#include <iostream>
//Opencv
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <chrono>
#include <strings.h>
#include <sstream>
#include <iomanip>

std::string type2str(int type) {
	std::string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}

std::string cvMat_description(const char* name, const cv::Mat& m) 
{
    std::stringstream sstr;
    double min, max;
    cv::minMaxLoc(m, &min, &max);
    sstr<<std::setw(20)<<name<<"\t - "<<m.size()<<"  "<<type2str(m.type())<<" ["<<min<<","<<max<<"]";
    return sstr.str();
}


//Dehaze image
cv::Mat dehaze(const cv::Mat& image, const cv::Mat& transmittance, double min, double max)
{
    std::chrono::time_point<std::chrono::system_clock> start;

    start = std::chrono::system_clock::now();
    
    cv::Mat colored_transmittance;
    cv::normalize(transmittance, colored_transmittance, min*255.0, max*255.0, cv::NORM_MINMAX, -1);
    cv::Mat1b atmosphere_mask  = (colored_transmittance <= 255.0*(min + 0.1*(max-min)));
    cv::Scalar atmosphere = cv::mean(image, atmosphere_mask);
    atmosphere/=255.0f;
    
    cv::cvtColor(colored_transmittance, colored_transmittance, CV_GRAY2RGB);
    cv::Mat num;
    cv::multiply(atmosphere, (colored_transmittance*(-1.0) + cv::Scalar(255.0,255.0,255.0)), num  , 1, CV_32FC3);
    cv::Mat imagef;
    image.convertTo(imagef, CV_32FC3);
    cv::subtract(imagef, num, num);
    cv::max(num, cv::Scalar(0.0,0.0,0.0), num);
    cv::Mat sol;
    cv::divide(num, colored_transmittance, sol, 255.0f, CV_8UC3);

    std::cerr<<"Tiempo dehaze : "<<std::chrono::duration<double>(std::chrono::system_clock::now() - start).count()<<std::endl;
    return sol;
}


/*int main(int argc, char** argv) {
	cv::String nimage         = "skyline.jpg";
	cv::String ntransmittance = "skyline_transmittance.jpg";
	cv::String nresult        = "skyline_dehazed.jpg";
	double min_t = 0.05;
	double max_t = 0.95;

    for (int i = 1;i<argc;++i)
    {
    if (i<argc - 1) {
	if (strcmp("-input",argv[i])==0)               nimage         = argv[++i];
	else if (strcmp("-transmittance",argv[i])==0)  ntransmittance = argv[++i];
	else if (strcmp("-output",argv[i])==0)         nresult        = argv[++i];
	else if (strcmp("-min",argv[i])==0)            min_t          = atof(argv[++i]);
	else if (strcmp("-max",argv[i])==0)            max_t          = atof(argv[++i]);
    }
    }
 

    cv::Mat image = cv::imread(nimage,CV_LOAD_IMAGE_COLOR);
    
    cv::Mat transmittance = cv::imread(ntransmittance,CV_LOAD_IMAGE_GRAYSCALE);
    transmittance.convertTo(transmittance,CV_32FC1);
    
    cv::Mat f1= dehaze(image, transmittance, min_t, max_t);
   
    //cv::imwrite(nresult, f1);
    cv::resize(f1,f1,cv::Size(640,480));
    imshow("nresult",f1);
    
    cv::waitKey(0);
}*/
