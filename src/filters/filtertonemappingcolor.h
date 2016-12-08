#pragma once

#include "filter.h"

class FilterTonemappingColor : public Filter {

static void color_correct_channel(cv::Mat& c, const cv::Scalar& m, const cv::Mat& color_factor) 
{
	cv::Mat dc;
	cv::multiply(c-m, color_factor, dc, 1, CV_32F);
	c = m + dc;
//	cv::min(c, cv::Scalar(1.0), c);
	cv::max(c, cv::Scalar(0.0), c);
}

static float luminance_curve(float luminance, float b, float c, float dl, float dh)
{
	float ll = std::log(luminance);
	float al = (c*dl - 1.0)/dl;
	float ah = (c*ah - 1.0)/ah;
	if (ll<(b-dl))      return 0.0;
	else if (ll<b)      return 0.5*c*(ll - b)/(1 - al*(ll-b)) + 0.5;
	else if (ll<(b+dh)) return 0.5*c*(ll - b)/(1 + ah*(ll-b)) + 0.5;
	else                return 1.0;
}

public:
//Using this: https://www.cl.cam.ac.uk/~rkm38/pdfs/mantiuk08mgtmo.pdf
static cv::Mat tonemap(const cv::Mat& image, const cv::Mat& factor, const cv::Mat& color_saturation, float brightness, float contrast, float dl, float dh, float factor_effect, float color_effect)
{
    cv::Mat sol;

    std::vector<cv::Mat> channels;
    cv::split(image, channels);
    cv::Mat& B = channels[0];
    cv::Mat& G = channels[1];
    cv::Mat& R = channels[2];
    cv::Mat luminance = 0.27*R + 0.67*G + 0.06*B;

    //These below are for the color correction.
    cv::Mat r =  R/luminance;
    cv::Mat g =  G/luminance;
    cv::Mat b =  B/luminance;

    cv::Mat color_factor;
    cv::exp(color_effect*(color_saturation - cv::Scalar(0.5)), color_factor);

    color_correct_channel(r, cv::Scalar(1.0), color_factor);
    color_correct_channel(g, cv::Scalar(1.0), color_factor);
    color_correct_channel(b, cv::Scalar(1.0), color_factor);

    //These below are for luminance correction
    cv::Mat f;
    cv::exp(factor_effect*(factor - cv::Scalar(0.5,0.5,0.5)), f);
    cv::multiply(luminance, f, luminance, 1, CV_32F);
    cv::Mat corrected_luminance(luminance.rows, luminance.cols, CV_32F);
    for(int i=0; i<luminance.rows; i++) 
    	for(int j=0; j<luminance.cols; j++) 
		corrected_luminance.at<float>(i,j) = luminance_curve(luminance.at<float>(i,j), brightness, contrast, dl, dh);
    
  
   
    cv::multiply(r, corrected_luminance, R, 255.0, CV_32F); 
    cv::multiply(g, corrected_luminance, G, 255.0, CV_32F); 
    cv::multiply(b, corrected_luminance, B, 255.0, CV_32F); 
    cv::merge(channels, sol);
    
    cv::max(sol, cv::Scalar(0.0,0.0,0.0), sol);
    cv::min(sol, cv::Scalar(255.0,255.0,255.0), sol);

    sol.convertTo(sol, CV_8UC3);
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
			FloatValue("Brightness",   0.0f,-4.0f),
			FloatValue("Contrast",     1.0f,5.0f),
			FloatValue("Local effect", 0.0f,4.0f),
			FloatValue("Color effect", 0.0f,5.0f)
		}};    
	}

	std::vector<Stroke> strokes() const override
       	{    
		return std::vector<Stroke>{{
			Stroke("Preserve",       0.50,0, 0.50,1),
			Stroke("Darken",         0.05,0),
			Stroke("Light up",       1.00,0),
			Stroke("Desaturate",     0.25,1),
			Stroke("Vivid",          0.75,1)
		}};		
	}



	cv::Mat apply(const cv::Mat& input_image, 
			const std::vector<std::shared_ptr<cv::Mat>>& propagated_values,
			const std::vector<float>& float_values) const override
	{	
		auto  expectedlum   = propagated_values[0];
		auto  color         = propagated_values[1];
		float brightness    = float_values[0];
		float contrast      = float_values[1];
		float local_effect  = float_values[2];
		float color_effect  = float_values[3];
        
		double min, max;
		cv::minMaxLoc(input_image, &min, &max);
        	return tonemap(input_image, *expectedlum, *color, brightness, contrast, std::log(min+(max-min)*0.1), std::log(max-(max-min)*0.1), local_effect, color_effect);
	}

};
