#include "include/runners/ThreadTrackApp.hpp"
#include "include/render/drawings.hpp"
#include <thread>

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
    std::atomic<bool> endedMain(false);
    std::atomic<bool> endedThread(false);

    std::jthread tracker(std::bind(&ThreadTrackApp::trackerThread, this, std::ref(captureDevice), std::ref(endedMain), std::ref(endedThread)));

    do {
        if (endedThread) break;
        auto poppedFrameOpt = deQueue.tryPopFront();
        if (poppedFrameOpt) {
            poppedFrameOpt->copyTo(frame);
            cv::imshow("Scene", frame);
        }

        // Measure and display fps
        if (FPS_main.is_updated())
            std::cout << "FPS main: " << FPS_main.get() << std::endl;
        FPS_main.update();

    } while (cv::pollKey() != 27);

    endedMain = true;

    return 0;
}

void ThreadTrackApp::trackerThread(cv::VideoCapture& captureDevice, std::atomic<bool>& endedMain, std::atomic<bool>& endedThread) {
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

ThreadTrackApp::~ThreadTrackApp()
{
}