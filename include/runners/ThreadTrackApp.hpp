#pragma once

#include <opencv2/opencv.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "recognizers/FaceRecognizer.hpp"
#include "recognizers/RedRecognizer.hpp"
#include "concurrency/SyncedDeque.hpp"
#include "utils/FpsMeter.hpp"

class ThreadTrackApp {
public:
	ThreadTrackApp();
	bool init(void);
	int run(void);
	void tracker_worker();
	void gl_worker();
	~ThreadTrackApp();

private:
	SyncedDeque<cv::Mat> de_queue;
	std::atomic<bool> ended_main = false;
    std::atomic<bool> ended_tracker_thread = false;
	FaceRecognizer face_recognizer;
	RedRecognizer red_recognizer;
	cv::VideoCapture capture_device;
	cv::Mat static_image;
	cv::Mat warning_image;
	FpsMeter FPS_main;
	FpsMeter FPS_tracker;

	// GL stuff
	GLFWwindow* window = nullptr;
	std::atomic<bool> ended_gl = false;
};
