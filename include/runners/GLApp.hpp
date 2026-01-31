#pragma once

#include <optional>
#include <thread>

#include <opencv2/opencv.hpp>
#include "include/recognizers/FaceRecognizer.hpp"
#include "include/recognizers/RedRecognizer.hpp"
#include "include/utils/FpsMeter.hpp"
#include "include/concurrency/SyncedDeque.hpp"
#include "include/concurrency/Pool.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <scenes/IScene.hpp>
#include "render/SyncedTexture.hpp"

class GLApp {
public:
	GLApp();
	bool load_config(const std::string& filename);
	bool init();
	bool init_imgui(void);
	bool init_cv(void);
	bool run(void);
	bool run_cv();
	~GLApp();

	int windowWidth = 800;
	int windowHeight = 600;

private:
	// OpenCV
	typedef struct RecognizedData {
		std::unique_ptr<SyncedTexture> frame;
		std::vector<cv::Point2f> faces;
		cv::Point2f red;
	} RecognizedData;
	RecognizedData defaultRecognizedData;
	SyncedDeque<RecognizedData> deQueue;
	Pool<SyncedTexture> framePool;
	std::atomic<bool> endedMain = false;
	std::atomic<bool> endedThread = false;
	std::jthread trackthr;
	FaceRecognizer faceRecognizer;
	RedRecognizer redRecognizer;
	cv::VideoCapture captureDevice;
	int cameraWidth, cameraHeight;
	cv::Mat staticImage;
	cv::Mat warningImage;
	FpsMeter FPS_main;
	FpsMeter FPS_tracker;

	void trackerThread();

	// OpenGL
	GLFWwindow* window = nullptr;
	bool vsync_on = true;
	bool first_focused = false;
	bool antialiasing_on = true;
	bool fullscreen = false;

	GLFWwindow* trackerWorkerWindow = nullptr;
	int backupW, backupH, backupX, backupY;

	// Models
	std::unique_ptr<IScene> activeScene;
	bool sceneOn = false;

	// ImGUI
	bool imgui_full = true;
	std::string gl_version;
	std::string gl_profile;
	void show_crosshair();

	// callbacks
	static void glfw_error_callback(int error, const char* description);
	static void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

};
