#pragma once

#include <opencv2/opencv.hpp>

class FaceRecognizer {
public:
	FaceRecognizer();
	bool init(void);
	int run(void);
	~FaceRecognizer();


private:
	cv::CascadeClassifier classifier;
	cv::VideoCapture captureDevice;

	cv::Point2f find_face(cv::Mat& frame);
};