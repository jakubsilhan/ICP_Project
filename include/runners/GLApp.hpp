#pragma once

#include <opencv2/opencv.hpp>
#include "include/recognizers/FaceRecognizer.hpp"
#include "include/recognizers/RedRecognizer.hpp"
#include "include/utils/fps_meter.hpp"
#include "include/concurrency/SyncedDeque.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class GLApp {
public:
	GLApp();
	bool init(void);
	bool init_cv(void);
	bool run(void);
	bool run_cv();
	~GLApp();

private:
	// OpenCV
	SyncedDeque<cv::Mat> deQueue;
	std::atomic<bool> endedMain = false;
	std::atomic<bool> endedThread = false;
	std::jthread cvdispthr;
	std::jthread trackthr;
	FaceRecognizer faceRecognizer;
	RedRecognizer redRecognizer;
	cv::VideoCapture captureDevice;
	cv::Mat staticImage;
	cv::Mat warningImage;
	fps_meter FPS_main;
	fps_meter FPS_cvdisplay;
	fps_meter FPS_tracker;

	void cvdisplayThread();
	void trackerThread();


	// GL stuff
	GLFWwindow* window = nullptr;

	// callbacks
	static void glfw_error_callback(int error, const char* description);

};
