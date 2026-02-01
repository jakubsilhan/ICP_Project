#include "include/runners/TrackApp.hpp"
#include "include/render/Drawings.hpp"

TrackApp::TrackApp() {
    // Constructor
}

bool TrackApp::init() {
    face_recognizer.init();

    static_image = cv::imread("resources/lock.png");
    warning_image = cv::imread("resources/warning.jpg");

    if (!capture_device.open(0)) {
        std::cerr << "Error: Could not open camera.\n";
        return false;
    }
    return true;
}

int TrackApp::run() {
    cv::Mat frame;

    do {
        capture_device.read(frame);
        if (frame.empty()) {
            std::cerr << "Cam disconnected? End of video?" << std::endl;
            return -1;
        }

        // find face
        std::vector<cv::Point2f> centers = face_recognizer.find_face(frame);

        // Display logic
        switch (centers.size()) {
            // 1. No face -> static image
            case 0:
                cv::imshow("app", static_image);
                break;
            
            // 2. One face -> track "some" object (track red)
            case 1:
                draw_cross_normalized(frame, red_recognizer.find_red(frame), 30);
                cv::imshow("app", frame);
                break;

            // 3. More than one face -> display warning
            default:
                cv::imshow("app", warning_image);
        }

        // Measure and display fps
        if (FPS.is_updated())
            std::cout << "FPS: " << FPS.get() << std::endl;
        FPS.update();
        
    } while (cv::pollKey() != 27);

    return 0;
}

TrackApp::~TrackApp() {
}
