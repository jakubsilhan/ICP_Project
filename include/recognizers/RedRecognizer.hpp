#pragma once

#include <opencv2/opencv.hpp>
#include <string>

class RedRecognizer {
public:
	RedRecognizer();
	int run_static(std::string path);
	int run_video(std::string path);


private:
	cv::VideoCapture captureDevice;
	cv::Point2f find_red(cv::Mat& frame);
};