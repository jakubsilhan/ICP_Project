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
	bool run(void);
	~GLApp();

private:
	fps_meter FPS_main;

	// GL stuff
	GLFWwindow* window = nullptr;

	// callbacks
	static void glfw_error_callback(int error, const char* description);

};
