#pragma once

#include <memory>
#include <vector>
#include <thread>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>

#include "scenes/IScene.hpp"
#include "render/SyncedTexture.hpp"
#include "concurrency/SyncedDeque.hpp"
#include "concurrency/Pool.hpp"
#include "recognizers/FaceRecognizer.hpp"
#include "recognizers/RedRecognizer.hpp"
#include "utils/FpsMeter.hpp"

class GLApp {
public:
	GLApp();
	bool load_config(const std::string& filename);
	bool init();
	bool init_imgui(void);
	bool init_cv(void);
	bool run(void);
	~GLApp();

	int window_width = 800;
	int window_height = 600;

private:
	// OpenGL
	GLFWwindow* window = nullptr;
	bool vsync_on = true;
	bool first_focused = false;
	bool antialiasing_on = true;
	bool fullscreen = false;
	GLFWwindow* tracker_worker_window = nullptr;
	int backup_w, backup_h, backup_x, backup_y;

	// Models
	std::unique_ptr<IScene> active_scene;

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

	// Camera tracking
	typedef struct RecognizedData {
		std::unique_ptr<SyncedTexture> frame;
		std::vector<cv::Point2f> faces;
		cv::Point2f red;
	} RecognizedData;
	RecognizedData default_recognized_data;
	SyncedDeque<RecognizedData> de_queue;
	Pool<SyncedTexture> frame_pool;
	std::atomic<bool> ended_main = false;
	std::atomic<bool> ended_tracker_thread = false;
	std::jthread tracker_thread;
	FaceRecognizer face_recognizer;
	RedRecognizer red_recognizer;
	cv::VideoCapture capture_device;
	int camera_width, camera_height;

	void tracker_worker();

	// FPS tracker
	FpsMeter FPS_main;
	FpsMeter FPS_tracker;

};
