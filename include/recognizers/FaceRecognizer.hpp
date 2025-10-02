#pragma once

#include <opencv2/opencv.hpp>

class FaceRecognizer {
public:
	FaceRecognizer();
	bool init(void);
	int run(void);
	~FaceRecognizer();
	std::vector<cv::Point2f> find_face(cv::Mat& frame);


private:
	cv::CascadeClassifier classifier;
	cv::VideoCapture captureDevice;
};