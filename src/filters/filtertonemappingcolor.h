#pragma once

#include "filter.h"

class FilterTonemappingColor : public Filter {

static cv::Mat tonemap(const cv::Mat& image, const cv::Mat& factor, const cv::Mat& color_saturation, double a, double lum_white, double factor_effect, double color_effect)
{
    cv::Mat sol;
    cv::Mat XYZ;
    cv::cvtColor(image, XYZ, CV_BGR2XYZ);

    std::vector<cv::Mat> channels;
    cv::split(XYZ, channels);
    cv::Mat& X = channels[0];
    cv::Mat& Y = channels[1];
    cv::Mat& Z = channels[2];
    cv::Mat  x = X/(X+Y+Z);
    cv::Mat  y = Y/(X+Y+Z);
    cv::Mat  z = Z/(X+Y+Z);


    std::cerr<<"lum_white="<<lum_white<<std::endl;
    print_info("Input", image);
    print_info("X", X); 
    print_info("Y", Y); 
    print_info("Z", Z);
    print_info("x", x);
    print_info("y", x);
    print_info("z", z); 
    
    cv::Mat log_luminance;
    cv::log(Y, log_luminance);
//    cv::Mat positive = Y > 0;
    double log_mean = std::exp(cv::mean(log_luminance, Y > 0)[0]);
    
    std::cerr<<"log_mean="<<log_mean<<std::endl;
    cv::Mat al = (a/log_mean)*Y;
    print_info("al",al);
    cv::Mat lw;
    cv::exp(-factor_effect*(factor - cv::Scalar(0.5,0.5,0.5)), lw);
    print_info("lw",lw);
    cv::Mat lw2;
    cv::multiply(lw, lw, lw2, lum_white*lum_white, CV_32F);
    print_info("lw2",lw2);
    cv::Mat ld;
    cv::divide(al,lw2,ld,1,CV_32F);
    print_info("ld",ld);
    ld = (cv::Scalar(1.0,1.0,1.0) + ld)/(cv::Scalar(1.0,1.0,1.0) + al);
    cv::multiply(al,ld,Y,1.0,CV_32F);

    cv::Mat color_factor;
    cv::exp(-color_effect*(color_saturation - cv::Scalar(0.5,0.5,0.5)), color_factor);
    print_info("color_factor",color_factor);
    cv::multiply(x, color_factor, x, 1, CV_32F);
    cv::min(x, cv::Scalar(1.0), x);
//    cv::multiply(y, color_factor, y, 1, CV_32F);
    cv::multiply(z, color_factor, z, 1, CV_32F);
    cv::min(z, cv::Scalar(1.0), z);
    y = cv::Scalar(1.0,1.0,1.0) - x - z;

    print_info("x", x);
    print_info("y", x);
    print_info("z", z); 
    
    cv::Mat normalization_factor;

    cv::divide(Y,y,normalization_factor, 1, CV_32F);
    cv::multiply(x,normalization_factor,X, 1, CV_32F);
    cv::multiply(z,normalization_factor,Z, 1, CV_32F);

    cv::merge(channels, sol);

    std::cerr<<"  vv  "<<std::endl;
    print_info("X", X); 
    print_info("Y", Y); 
    print_info("Z", Z);
    print_info("x", x);
    print_info("y", x);
    print_info("z", z); 

    cv::cvtColor(sol, sol, CV_XYZ2BGR);
    print_info("Output", sol);
    sol*=255.0;
    cv::max(sol, cv::Scalar(0.0,0.0,0.0), sol);
    cv::min(sol, cv::Scalar(255.0,255.0,255.0), sol);
    sol.convertTo(sol, CV_8UC3);

    print_info("Final", sol);
    std::cerr<<"-----"<<std::endl;
    return sol;
}


public:
	std::vector<std::string> propagatedValues() const override
       	{    
		return std::vector<std::string>{{
			std::string("Local exposure"),
			std::string("Color saturation")
		}};    
	
	}

	std::vector<FloatValue> floatValues() const override
       	{    
		return std::vector<FloatValue>{{
			FloatValue("Exposure",   -2.5f,2.5f),
			FloatValue("Local effect",0.0f,5.0f),
			FloatValue("Color effect",0.0f,5.0f)
		}};    
	}

	std::vector<Stroke> strokes() const override
       	{    
		return std::vector<Stroke>{{
			Stroke("Preserve",       0.50,0, 0.50,1),
			Stroke("Darken",         0.05,0),
			Stroke("Light up",       1.00,0),
			Stroke("Desaturate",     0.05,1),
			Stroke("Vivid",          1.00,1)
		}};		
	}



	cv::Mat apply(const cv::Mat& input_image, 
			const std::vector<std::shared_ptr<cv::Mat>>& propagated_values,
			const std::vector<float>& float_values) const override
	{	
		auto  expectedlum   = propagated_values[0];
		auto  color         = propagated_values[1];
		float exposure      = float_values[0];
		float local_effect  = float_values[1];
		float color_effect  = float_values[2];
        
		double min, max;
		cv::minMaxLoc(input_image, &min, &max);
        	return tonemap(input_image, *expectedlum, *color, 0.18, 0.5*(max+min)*std::exp(-exposure), local_effect, color_effect);
	}

};
