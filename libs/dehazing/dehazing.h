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

std::string type2str(int type);

std::string cvMat_description(const char* name, const cv::Mat& m);


//Dehaze image
cv::Mat dehaze(const cv::Mat& image, const cv::Mat& transmittance, double min, double max);
