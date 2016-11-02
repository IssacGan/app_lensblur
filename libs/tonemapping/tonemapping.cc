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

#include "tonemapping/tonemapping.h"
//#include "imageHDR.h"
/* utility for reading and writing Ward's rgbe image format.
   See rgbe.txt file for more details.
*/




/* default error routine.  change this to change error handling */
/*int rgbe_error(int rgbe_error_code, const char *msg)
{
  switch (rgbe_error_code) {
  case rgbe_read_error:
    perror("RGBE read error");
    break;
  case rgbe_write_error:
    perror("RGBE write error");
    break;
  case rgbe_format_error:
    fprintf(stderr,"RGBE bad file format: %s\n",msg);
    break;
  default:
  case rgbe_memory_error:
    fprintf(stderr,"RGBE error: %s\n",msg);
  }
  return RGBE_RETURN_FAILURE;
}*/




//Dehaze image
cv::Mat tonemap(const cv::Mat& image, const cv::Mat& factor, double a, double lum_white)
{
    std::chrono::time_point<std::chrono::system_clock> start;
    start = std::chrono::system_clock::now();
    
    std::vector<cv::Mat> channels;
    cv::split(image, channels);

    cv::Mat luminance = 0.27*channels[0] + 0.67*channels[1] + 0.06*channels[2];
    
    double lmin, lmax;
    cv::minMaxLoc(luminance, &lmin, &lmax);
    double lw_ = lum_white*2.0/lmax;

    cv::Mat log_luminance;
    cv::log(luminance, log_luminance);
    cv::Mat positive = luminance > 0;
    double log_mean = std::exp(cv::mean(log_luminance, positive)[0]);
    
    cv::Mat al = (a/log_mean)*luminance;
    cv::Mat lw = lw_*factor;
    cv::Mat lw2;
    cv::multiply(lw, lw, lw2, 1, CV_32F);
    cv::Mat ld;
    cv::divide(al,lw2,ld,1,CV_32F);
    ld = (cv::Scalar(1.0,1.0,1.0) + ld)/(cv::Scalar(1.0,1.0,1.0) + al);
    cv::multiply(al,ld,ld,255.0,CV_32F);
    cv::multiply(luminance, lw, luminance,1,CV_32F);
    cv::Mat f;
    cv::divide(ld,luminance, f, 1, CV_32F);
    cv::Mat sol;
    cv::cvtColor(f, f, CV_GRAY2RGB);
    cv::multiply(f, image, sol, 1, CV_32FC3);
    cv::max(sol, cv::Scalar(0.0,0.0,0.0), sol);
    cv::min(sol, cv::Scalar(255.0,255.0,255.0), sol);
    sol.convertTo(sol, CV_8UC3);

    std::cerr<<"Tiempo tonemap : "<<std::chrono::duration<double>(std::chrono::system_clock::now() - start).count()<<std::endl;
    return sol;
}


/*int main(int argc, const char** argv) {
	const char* nimage   = "memorial.hdr";
	cv::String nlumwhite = "memorial_lumwhite.jpg";
	cv::String nresult   = "memorial_tonemapped.jpg";
	double a        = 0.18;
	double lumwhite = 1.0;

    for (int i = 1;i<argc;++i)
    {
    if (i<argc - 1) {
	if (strcmp("-input",argv[i])==0)               nimage    = argv[++i];
	else if (strcmp("-lumwhite-map",argv[i])==0)   nlumwhite = argv[++i];
	else if (strcmp("-output",argv[i])==0)         nresult   = argv[++i];
	else if (strcmp("-a",argv[i])==0)              a         = atof(argv[++i]);
	else if (strcmp("-lumwhite",argv[i])==0)       lumwhite  = atof(argv[++i]);
    }
    }
 

    cv::Mat image = load_hdr(nimage);
    
    cv::Mat lumwhite_map = cv::imread(nlumwhite,CV_LOAD_IMAGE_GRAYSCALE);
    lumwhite_map.convertTo(lumwhite_map,CV_32FC1);
    
    cv::Mat f1= tonemap(image, lumwhite_map, a, lumwhite);
   
    cv::imwrite(nresult, f1); 
}*/
