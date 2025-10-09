#include "include/runners/ThreadTrackApp.hpp"
#include "include/render/drawings.hpp"

ThreadTrackApp::ThreadTrackApp() {
    // Constructor
}

bool ThreadTrackApp::init() {
    faceRecognizer.init();

    staticImage = cv::imread("resources/lock.png");
    warningImage = cv::imread("resources/warning.jpg");

    if (!captureDevice.open(0)) {
        std::cerr << "Error: Could not open camera.\n";
        return false;
    }
    return true;
}

int ThreadTrackApp::run() {
    cv::Mat frame;

    do {
        captureDevice.read(frame);
        if (frame.empty()) {
            std::cerr << "Cam disconnected? End of video?" << std::endl;
            return -1;
        }

        // find face
        std::vector<cv::Point2f> centers = faceRecognizer.find_face(frame);

        // Display logic
        switch (centers.size()) {
            // 1. No face -> static image
        case 0:
            cv::imshow("app", staticImage);
            break;

            // 2. One face -> track "some" object (track red)
        case 1:
            draw_cross_normalized(frame, redRecognizer.find_red(frame), 30);
            cv::imshow("app", frame);
            break;

            // 3. More than one face -> display warning
        default:
            cv::imshow("app", warningImage);
        }

        // Measure and display fps
        if (FPS.is_updated())
            std::cout << "FPS: " << FPS.get() << std::endl;
        FPS.update();

    } while (cv::pollKey() != 27);

    return 0;
}

ThreadTrackApp::~ThreadTrackApp()
{
}