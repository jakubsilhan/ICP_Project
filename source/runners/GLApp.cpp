#include <thread>

#include "include/runners/GLApp.hpp"
#include "include/render/drawings.hpp"
#include "include/render/Triangle.hpp"
#include "include/utils/GlErrCallback.hpp"

GLApp::GLApp()
{
}

bool GLApp::init() {
    // GLFW initialization
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Error: Could not initialize GLFW.\n";
        return false;
    }
    // Version specifications
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Window creation
    window = glfwCreateWindow(800, 600, "OpenGL context", NULL, NULL);
    if (!window) {
        std::cerr << "Error: Could not create GLFW window. \n";
        glfwTerminate();
        return false;
    }

    // Context must be assigned per thread (and should not be assigned in multiple ones
    glfwMakeContextCurrent(window);

    // Requires assigned context
    if (glewInit() != GLEW_OK) {
        std::cerr << "Error: Could not initialize GLEW.\n";
        return false;
    }

    if (GLEW_ARB_debug_output) {
        glDebugMessageCallback(MessageCallback, 0);
        glEnable(GL_DEBUG_OUTPUT);

        std::cout << "GL_DEBUG enabled" << std::endl;
    }
    else {
        std::cout << "GL_DEBUG NOT SUPPORTED!" << std::endl;
    }

    // Requires initialized glew
    if (!GLEW_ARB_direct_state_access) {
        std::cerr << "Error: DSA (Direct System Access) is not available.\n";
        return false;
    }

    return true;
}

bool GLApp::run() {

    // Set background color
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Prepare triangle
    Triangle triangle;
    if (!triangle.init()) {
        std::cerr << "Error: Could not initialize triangle. \n";
        return false;
    }

    // Set triangle color
    triangle.setColor(1, 0, 0, 1);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        triangle.draw();

        // Switch background and foreground buffers (rendering is always done in background first)
        glfwSwapBuffers(window);

        // Check key, mouse events
        glfwPollEvents();

        // Sleep to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    return true;
}

#pragma region OpenCV
bool GLApp::init_cv() {
    // Classic tracker initializations
    faceRecognizer.init();

    staticImage = cv::imread("resources/lock.png");
    warningImage = cv::imread("resources/warning.jpg");

    if (!captureDevice.open(0)) {
        std::cerr << "Error: Could not open camera.\n";
        return false;
    }
}

bool GLApp::run_cv() {
    cvdispthr = std::jthread(&GLApp::cvdisplayThread, this);
    trackthr = std::jthread(&GLApp::trackerThread, this);
}

void GLApp::cvdisplayThread() {
    cv::Mat frame;

    do {
        if (endedThread) break;

        if (!deQueue.empty()) {
            deQueue.popFront().copyTo(frame);
            cv::imshow("Scene", frame);
        }

        // Measure and display fps
        if (FPS_main.is_updated())
            std::cout << "FPS main: " << FPS_main.get() << std::endl;
        FPS_main.update();

    } while (cv::waitKey(10) != 27);

    endedThread = true;
}

void GLApp::trackerThread() {
    cv::Mat frame;

    while (!endedMain && !endedThread) {
        captureDevice.read(frame);
        if (frame.empty()) {
            std::cerr << "Cam disconnected? End of video?" << std::endl;
            endedThread = true;
            return;
        }

        // find face
        std::vector<cv::Point2f> centers = faceRecognizer.find_face(frame);

        // Display logic
        switch (centers.size()) {
            // 1. No face -> static image
        case 0:
            deQueue.pushBack(staticImage);
            break;

            // 2. One face -> track "some" object (track red)
        case 1:
            draw_cross_normalized(frame, centers.front(), 30, CV_RGB(0, 255, 0));
            draw_cross_normalized(frame, redRecognizer.find_red(frame), 30);
            deQueue.pushBack(frame);
            break;

            // 3. More than one face -> display warning
        default:
            deQueue.pushBack(warningImage);
        }

        if (FPS_tracker.is_updated())
            std::cout << "FPS tracker: " << FPS_tracker.get() << std::endl;
        FPS_tracker.update();
    }
}
#pragma endregion

// Callbacks
void GLApp::glfw_error_callback(int error, const char* description)
{
    std::cerr << "GLFW error: " << description << std::endl;
}

GLApp::~GLApp()
{
    if (window) {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}
