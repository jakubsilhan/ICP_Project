#pragma once

#include <opencv2/opencv.hpp>
#include "include/recognizers/FaceRecognizer.hpp"
#include "include/recognizers/RedRecognizer.hpp"
#include "include/fps_meter.hpp"

class TrackApp {
public:
	TrackApp();
	bool init(void);
	int run(void);
	~TrackApp();

private:
	FaceRecognizer faceRecognizer;
	RedRecognizer redRecognizer;
	cv::VideoCapture captureDevice;
	cv::Mat staticImage;
	cv::Mat warningImage;
	fps_meter FPS;
};
