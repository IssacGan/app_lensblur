#ifndef _BLUR_H_
#define _BLUR_H_

//#include <iostream>
//Opencv
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
//#include <chrono>
//#include <strings.h>


/**
 *
 * maxBlur in number of pixels
 **/
std::vector<cv::Mat> create_blurred(const cv::Mat& input, double maxBlur, int nbins);
cv::Mat interpolate_in_vector(const std::vector<cv::Mat>& source,const cv::Mat& interpolant);
std::string type2str(int type) ;
float blur_size_from_distance(float distance, float focal_distance, float focal_length, float aperture, bool linear);

/**
 * Esto no lo usamos ya, no son layers sino iteraciones hasta la solucion, así que
 * hemos copiado y pegado el código
 **/
std::vector<cv::Mat> create_depth_layers_old(const cv::Mat& input, const cv::Mat& depth, 
                                             int nbins, float focal_distance, float focal_length, float aperture);

/**
 * Esto está en progreso, no funciona
 **/
std::vector<cv::Mat> create_depth_layers_with_interpolation(const cv::Mat& input, const cv::Mat& depth, 
		int nbins, float focal_distance, float focal_length, float aperture);
//Blur image
cv::Mat blur_image_depth(const cv::Mat& image, const cv::Mat& depth, 
                         int nbins, float focal_distance, float focal_length, float aperture, bool linear);


//Blur image
cv::Mat blur_image_focal_distance(const cv::Mat& image, const cv::Mat& depth, 
		int nbins, float focal_distance, float focal_length, float aperture, bool linear);


void  adolfoBlur();

#endif
