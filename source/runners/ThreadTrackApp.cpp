#include "include/runners/ThreadTrackApp.hpp"
#include "include/render/Drawings.hpp"
#include "include/render/TriangleOld.hpp"
#include <thread>

ThreadTrackApp::ThreadTrackApp() {
    // Constructor
}

bool ThreadTrackApp::init() {
    // Classic tracker initializations
    faceRecognizer.init();

    staticImage = cv::imread("resources/lock.png");
    warningImage = cv::imread("resources/warning.jpg");

    if (!captureDevice.open(0)) {
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

    endedMain = false;

    std::jthread tracker(&ThreadTrackApp::trackerThread, this);
    std::jthread gl(&ThreadTrackApp::glThread, this);

    do {
        if (endedThread || endedGl) break;

        if (!deQueue.empty()) {
            deQueue.popFront().copyTo(frame);
            cv::imshow("Scene", frame);
        }

        // Measure and display fps
        if (FPS_main.is_updated())
            std::cout << "FPS main: " << FPS_main.get() << std::endl;
        FPS_main.update();

    } while (cv::waitKey(10) != 27);

    endedMain = true;

    return 0;
}

void ThreadTrackApp::trackerThread() {
    cv::Mat frame;

    while(!endedMain){
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
            deQueue.pushBack(std::move(staticImage));
            break;

            // 2. One face -> track "some" object (track red)
        case 1:
            draw_cross_normalized(frame, centers.front(), 30, CV_RGB(0, 255, 0));
            draw_cross_normalized(frame, redRecognizer.find_red(frame), 30);
            deQueue.pushBack(std::move(frame));
            break;

            // 3. More than one face -> display warning
        default:
            deQueue.pushBack(std::move(warningImage));
        }

        if (FPS_tracker.is_updated())
            std::cout << "FPS tracker: " << FPS_tracker.get() << std::endl;
        FPS_tracker.update();
    }
}

void ThreadTrackApp::glThread() {
    // Context must be assigned per thread (and should not be assigned in multiple ones)
    glfwMakeContextCurrent(window);

    // Requires assigned context
    if (glewInit() != GLEW_OK) {
        std::cerr << "Error: Could not initialize GLEW.\n";
        endedGl = true;
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
        endedGl = true;
        return;
    }

    // Set triangle color
    triangle.setColor(1, 0, 0, 1);

    while (!glfwWindowShouldClose(window) && !endedMain) {
        glClear(GL_COLOR_BUFFER_BIT);
        triangle.draw();

        // Switch background and foreground buffers (rendering is always done in background first)
        glfwSwapBuffers(window);

        // Check key, mouse events
        glfwPollEvents();

        // Sleep to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    endedGl = true;
}

ThreadTrackApp::~ThreadTrackApp()
{
    if (window) {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}