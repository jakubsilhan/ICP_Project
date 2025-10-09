#pragma once

#include <opencv2/opencv.hpp>
#include "include/recognizers/FaceRecognizer.hpp"
#include "include/recognizers/RedRecognizer.hpp"
#include "include/utils/fps_meter.hpp"
#include "include/concurrency/SyncedDeque.hpp"

class ThreadTrackApp {
public:
	ThreadTrackApp();
	bool init(void);
	int run(void);
	void trackerThread(cv::VideoCapture& capture, std::atomic<bool>& endedMain, std::atomic<bool>& endedThread);
	~ThreadTrackApp();

private:
	SyncedDeque<cv::Mat> deQueue;
	FaceRecognizer faceRecognizer;
	RedRecognizer redRecognizer;
	cv::VideoCapture captureDevice;
	cv::Mat staticImage;
	cv::Mat warningImage;
	fps_meter FPS_main;
	fps_meter FPS_tracker;
};
