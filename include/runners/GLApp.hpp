#pragma once

#include <thread>

#include <opencv2/opencv.hpp>
#include "include/recognizers/FaceRecognizer.hpp"
#include "include/recognizers/RedRecognizer.hpp"
#include "include/utils/fps_meter.hpp"
#include "include/concurrency/SyncedDeque.hpp"
#include "include/concurrency/Pool.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "render/Triangle.hpp"

class GLApp {
public:
	GLApp();
	bool load_config(const std::string& filename);
	bool init(void);
	bool init_imgui(void);
	bool init_cv(void);
	bool run(void);
	bool run_cv();
	~GLApp();

private:
	// OpenCV
	typedef struct RecognizedData {
		std::unique_ptr<cv::Mat> frame;
		std::vector<cv::Point2f> faces;
		cv::Point2f red;
	} RecognizedData;
	SyncedDeque<RecognizedData> deQueue;
	Pool<cv::Mat> framePool;
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


	// OpenGL
	GLFWwindow* window = nullptr;
	int windowWidth = 800;
	int windowHeight = 600;
	bool vsync_on = true;

	// Models
	std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> shader_library;
	std::unordered_map<std::string, std::shared_ptr<Mesh>> mesh_library;
	std::unordered_map<std::string, Model> scene;
	int triangleColorIndex = 0;
	void init_assets(void);

	// ImGUI
	bool imgui_on = true;

	// callbacks
	static void glfw_error_callback(int error, const char* description);
	static void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

};
