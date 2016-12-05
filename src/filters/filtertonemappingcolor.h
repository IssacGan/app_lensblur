#pragma once

#include "filter.h"

class FilterTonemappingColor : public Filter {

static cv::Mat tonemap(const cv::Mat& image, const cv::Mat& factor, const cv::Mat& color_saturation, double a, double lum_white, double factor_effect, double color_effect)
{
    cv::Mat sol;
    cv::Mat image_lab;
    cv::cvtColor(image, image_lab, CV_BGR2YCrCb);

    std::vector<cv::Mat> channels;
    cv::split(image_lab, channels);
    cv::Mat luminance = channels[0];

    std::cerr<<"lum_white="<<lum_white<<std::endl;
    print_info("Input", image);
    print_info("L", channels[0]); 
    print_info("a", channels[1]); 
    print_info("b", channels[2]); 
    
    cv::cvtColor(image_lab, sol, CV_YCrCb2BGR);
    print_info("ReInput", sol);
    double lmin, lmax;
    cv::minMaxLoc(luminance, &lmin, &lmax);
    double lw_ = lum_white; //lmax;
    cv::Mat log_luminance;
    cv::log(luminance, log_luminance);
    cv::Mat positive = luminance > 0;
    double log_mean = std::exp(cv::mean(log_luminance, positive)[0]);
    
    cv::Mat al = (a/log_mean)*luminance;
    cv::Mat lw;
    cv::exp(-factor_effect*(factor - cv::Scalar(0.5,0.5,0.5)), lw);
    lw*=lw_;
    cv::Mat lw2;
    cv::multiply(lw, lw, lw2, 1, CV_32F);
    cv::Mat ld;
    cv::divide(al,lw2,ld,1,CV_32F);
    ld = (cv::Scalar(1.0,1.0,1.0) + ld)/(cv::Scalar(1.0,1.0,1.0) + al);
    cv::multiply(al,ld,ld,1.0,CV_32F);
    cv::multiply(luminance, lw, luminance,1,CV_32F);
    cv::Mat fL;
    cv::divide(ld,luminance, fL, 1, CV_32F);

    cv::Mat fab;
    cv::exp(-color_effect*(color_saturation - cv::Scalar(0.5,0.5,0.5)), fab);

    cv::multiply(fL,  channels[0], channels[0], 255, CV_32F);
    cv::multiply(fab, channels[1], channels[1], 1, CV_32F);
    cv::multiply(fab, channels[2], channels[2], 1, CV_32F);
    cv::merge(channels, sol);

    std::cerr<<"  vv  "<<std::endl;
    print_info("L", channels[0]); 
    print_info("a", channels[1]); 
    print_info("b", channels[2]); 
    print_info("Lab", sol);

    cv::cvtColor(sol, sol, CV_YCrCb2BGR);
    print_info("Output", sol);
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
		std::cerr<<"min="<<min<<" - max="<<max<<" - exposure="<<exposure<<" - lw="<<0.5*(max+min)*std::exp(-exposure)<<std::endl;

        	return tonemap(input_image, *expectedlum, *color, 0.18, 0.5*(max+min)*std::exp(-exposure), local_effect, color_effect);
	}

};
