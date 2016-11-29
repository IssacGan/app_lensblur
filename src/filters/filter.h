#pragma once

#include <vector>
#include <string>
#include <tuple>
#include <opencv2/core/core.hpp>


class Filter {
protected:
	static void print_info(const char* name, const cv::Mat& image) 
	{
		double min, max;
		cv::minMaxLoc(image, &min, &max);

		std::cerr<<std::setw(20)<<name<<"\t- "<<image.cols<<"x"<<image.rows<<"x"<<image.channels()<<" - ["<<min<<","<<max<<"]"<<std::endl;
	}

public:
	class FloatValue {
		std::string _name;
		float _min, _max;
		int _pickable_from_channel;
	public:
		FloatValue(const std::string& name = std::string(), float min = 0.0f, float max = 1.0f, int pickable_from_channel = -1) :
			_name(name), _min(min), _max(max), _pickable_from_channel(pickable_from_channel) { }
		FloatValue(const char* name, float min, float max, int pickable_from_channel = -1) : 
			FloatValue(std::string(name), min, max, pickable_from_channel) { }
		const std::string name() const { return _name; }
		float min() const { return _min; }
		float max() const { return _max; }
		bool is_pickable() const { return _pickable_from_channel >= 0; }
		int  channel_to_pick_from() const { return _pickable_from_channel; }
	};

	virtual std::vector<std::string> propagatedValues() const
       	{    return std::vector<std::string>(0);    }
	virtual std::vector<FloatValue> floatValues() const
       	{    return std::vector<FloatValue>(0);    }

	virtual cv::Mat apply(const cv::Mat& input_image, 
			const std::vector<std::shared_ptr<cv::Mat>>& propagated_values,
			const std::vector<float>& float_values) const = 0;

};
