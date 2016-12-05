#pragma once

#include "filter.h"

class FilterTonemapping : public Filter {

static cv::Mat tonemap(const cv::Mat& image, const cv::Mat& factor, double a, double lum_white, double factor_effect)
{
    std::vector<cv::Mat> channels;
    cv::split(image, channels);

    cv::Mat luminance = 0.27*channels[0] + 0.67*channels[1] + 0.06*channels[2];
    
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
    cv::multiply(al,ld,ld,255.0,CV_32F);
    cv::multiply(luminance, lw, luminance,1,CV_32F);
    cv::Mat f;
    cv::divide(ld,luminance, f, 1, CV_32F);
    cv::Mat sol;
    cv::cvtColor(f, f, CV_GRAY2RGB);
    cv::multiply(f, image, sol, 255, CV_32FC3);
    cv::max(sol, cv::Scalar(0.0,0.0,0.0), sol);
    cv::min(sol, cv::Scalar(255.0,255.0,255.0), sol);
    sol.convertTo(sol, CV_8UC3);

    return sol;
}


public:
	std::vector<std::string> propagatedValues() const override
       	{    
		return std::vector<std::string>{{
			std::string("Local exposure")
		}};    
	
	}

	std::vector<FloatValue> floatValues() const override
       	{    
		return std::vector<FloatValue>{{
			FloatValue("Exposure",-5.0f,5.0f),
			FloatValue("Local effect",0.0f,5.0f)
		}};    
	}

	std::vector<Stroke> strokes() const override
       	{    
		return std::vector<Stroke>{{
			Stroke("Darken",         0.05),
			Stroke("Preserve light", 0.50),
			Stroke("Light up",       1.00)
		}};		
	}



	cv::Mat apply(const cv::Mat& input_image, 
			const std::vector<std::shared_ptr<cv::Mat>>& propagated_values,
			const std::vector<float>& float_values) const override
	{	
		auto  expectedlum   = propagated_values[0];
		float exposure  = float_values[0];
		float local_effect  = float_values[1];
        
		double min, max;
		cv::minMaxLoc(input_image, &min, &max);

        	return tonemap(input_image, *expectedlum, 0.18, (0.8*max+0.2*min)*std::exp(-exposure), local_effect);
	}

};
