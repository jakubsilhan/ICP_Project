#pragma once

#include <opencv2/opencv.hpp>

#include "recognizers/FaceRecognizer.hpp"
#include "recognizers/RedRecognizer.hpp"
#include "utils/FpsMeter.hpp"

class TrackApp {
public:
	TrackApp();
	bool init(void);
	int run(void);
	~TrackApp();

private:
	FaceRecognizer face_recognizer;
	RedRecognizer red_recognizer;
	cv::VideoCapture capture_device;
	cv::Mat static_image;
	cv::Mat warning_image;
	FpsMeter FPS;
};
