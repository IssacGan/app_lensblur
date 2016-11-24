#pragma once

#include "filter.h"

class FilterDehazing : public Filter {
static void print_info(const char* name, const cv::Mat& image) 
{
	double min, max;
	cv::minMaxLoc(image, &min, &max);

	std::cerr<<std::setw(20)<<name<<"\t- "<<image.cols<<"x"<<image.rows<<"x"<<image.channels()<<" - ["<<min<<","<<max<<"]"<<std::endl;
}


	static cv::Mat dehaze(const cv::Mat& image, const cv::Mat& transmittance, double min, double max)
	{
		print_info("Image",image);
		print_info("Transmittance",transmittance);
	    cv::Mat colored_transmittance;
	    cv::normalize(transmittance, colored_transmittance, min*255.0, max*255.0, cv::NORM_MINMAX, -1);
	    cv::Mat1b atmosphere_mask  = (colored_transmittance <= 255.0*(min + 0.1*(max-min)));
	    cv::Scalar atmosphere = cv::mean(image, atmosphere_mask);
	    atmosphere/=255.0f;
	    
		print_info("C. Transmittance",colored_transmittance);
	    cv::cvtColor(colored_transmittance, colored_transmittance, CV_GRAY2RGB);
	    cv::Mat num;
	    cv::multiply(atmosphere, (colored_transmittance*(-1.0) + cv::Scalar(255.0,255.0,255.0)), num  , 1, CV_32FC3);
	    cv::Mat imagef;
	    image.convertTo(imagef, CV_32FC3);
	    cv::subtract(imagef, num, num);
	    cv::max(num, cv::Scalar(0.0,0.0,0.0), num);
	    cv::Mat sol;
	    cv::divide(num, colored_transmittance, sol, 255.0f, CV_8UC3);

	    return sol;
	}

public:
	std::vector<std::string> propagatedValues() const override
       	{    
		return std::vector<std::string>{{
			std::string("Transmittance")
		}};    
	
	}

	std::vector<std::tuple<std::string, float, float>> floatValues() const override
       	{    
		return std::vector<std::tuple<std::string, float, float>>{{
			std::make_tuple(std::string("Effect"),0.0f,1.0f)
		}};    
	}

	cv::Mat apply(const cv::Mat& input_image, 
			const std::vector<std::shared_ptr<cv::Mat>>& propagated_values,
			const std::vector<float>& float_values) const override
	{	
		auto  transmittance = propagated_values[0];
		float intensity     = float_values[0];
	        double min, max;
		cv::minMaxLoc(*transmittance, &min, &max);
		
		double min_t = (intensity*min) + (1.0 - intensity)*max;//0.05;
		double max_t = max;//0.95;
        
        	return dehaze(input_image, *transmittance, min_t, max_t);
	}

};
