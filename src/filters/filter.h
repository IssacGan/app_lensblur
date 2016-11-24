#pragma once

#include <vector>
#include <string>
#include <tuple>
#include <opencv2/core/core.hpp>

class Filter {

public:
	virtual std::vector<std::string> propagatedValues() const
       	{    return std::vector<std::string>(0);    }
	virtual std::vector<std::tuple<std::string, float, float>> floatValues() const
       	{    return std::vector<std::tuple<std::string, float, float>>(0);    }

	virtual cv::Mat apply(const cv::Mat& input_image, 
			const std::vector<std::shared_ptr<cv::Mat>>& propagated_values,
			const std::vector<float>& float_values) const = 0;

};
