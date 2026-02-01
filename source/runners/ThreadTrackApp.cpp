#include "include/runners/ThreadTrackApp.hpp"
#include "include/render/Drawings.hpp"
#include "include/render/TriangleOld.hpp"
#include <thread>

ThreadTrackApp::ThreadTrackApp() {
    // Constructor
}

bool ThreadTrackApp::init() {
    // Classic tracker initializations
    face_recognizer.init();

    static_image = cv::imread("resources/lock.png");
    warning_image = cv::imread("resources/warning.jpg");

    if (!capture_device.open(0)) {
        std::cerr << "Error: Could not open camera.\n";
        return false;
    }

    // GLFW initialization
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
        std::cerr << "Error: Could not creeate GLFW window. \n";
        glfwTerminate();
        return false;
    }

    return true;
}

int ThreadTrackApp::run() {
    cv::Mat frame;

    ended_main = false;

    std::jthread tracker(&ThreadTrackApp::tracker_worker, this);
    std::jthread gl(&ThreadTrackApp::gl_worker, this);

    do {
        if (ended_tracker_thread || ended_gl) break;

        if (!de_queue.empty()) {
            de_queue.pop_front().copyTo(frame);
            cv::imshow("Scene", frame);
        }

        // Measure and display fps
        if (FPS_main.is_updated())
            std::cout << "FPS main: " << FPS_main.get() << std::endl;
        FPS_main.update();

    } while (cv::waitKey(10) != 27);

    ended_main = true;

    return 0;
}

void ThreadTrackApp::tracker_worker() {
    cv::Mat frame;

    while(!ended_main){
        capture_device.read(frame);
        if (frame.empty()) {
            std::cerr << "Cam disconnected? End of video?" << std::endl;
            ended_tracker_thread = true;
            return;
        }

        // find face
        std::vector<cv::Point2f> centers = face_recognizer.find_face(frame);

        // Display logic
        switch (centers.size()) {
            // 1. No face -> static image
        case 0:
            static_image.copyTo(frame);            
            de_queue.push_back(std::move(frame));
            break;

            // 2. One face -> track "some" object (track red)
        case 1:
            draw_cross_normalized(frame, centers.front(), 30, CV_RGB(0, 255, 0));
            draw_cross_normalized(frame, red_recognizer.find_red(frame), 30);
            de_queue.push_back(std::move(frame));
            break;

            // 3. More than one face -> display warning
        default:
            warning_image.copyTo(frame);
            de_queue.push_back(std::move(frame));
        }

        if (FPS_tracker.is_updated())
            std::cout << "FPS tracker: " << FPS_tracker.get() << std::endl;
        FPS_tracker.update();
    }
}

void ThreadTrackApp::gl_worker() {
    // Context must be assigned per thread (and should not be assigned in multiple ones)
    glfwMakeContextCurrent(window);

    // Requires assigned context
    if (glewInit() != GLEW_OK) {
        std::cerr << "Error: Could not initialize GLEW.\n";
        ended_gl = true;
        return;
    }

    // Requires initialized glew
    if (!GLEW_ARB_direct_state_access)
        throw std::runtime_error("No DSA :-(");

    // Set background color
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Prepare triangle
    TriangleOld triangle;
    if (!triangle.init()) {
        std::cerr << "Error: Could not initialize triangle. \n";
        ended_gl = true;
        return;
    }

    // Set triangle color
    triangle.set_color(1, 0, 0, 1);

    while (!glfwWindowShouldClose(window) && !ended_main) {
        glClear(GL_COLOR_BUFFER_BIT);
        triangle.draw();

        // Switch background and foreground buffers (rendering is always done in background first)
        glfwSwapBuffers(window);

        // Check key, mouse events
        glfwPollEvents();

        // Sleep to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    ended_gl = true;
}

ThreadTrackApp::~ThreadTrackApp()
{
    if (window) {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}