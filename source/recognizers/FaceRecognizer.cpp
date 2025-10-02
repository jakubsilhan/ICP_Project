#include "include/recognizers/FaceRecognizer.hpp"
#include "include/render/drawings.hpp"

FaceRecognizer::FaceRecognizer() {
	// Constructor
}

bool FaceRecognizer::init() {
    if (!classifier.load("../resources/haarcascade_frontalface_default.xml")) {
        std::cerr << "Error: Could not load face classifier.\n";
        return false;
    }

    return true;
}

int FaceRecognizer::run() {
    cv::Mat frame; // captured frame

    if (!captureDevice.open(0)) {
        std::cerr << "Error: Could not open camera.\n";
        return false;
    }

    do {
        captureDevice.read(frame);
        if (frame.empty()) {
            std::cerr << "Cam disconnected? End of video?" << std::endl;
            return -1;
        }

        // find face
        std::vector<cv::Point2f> centers = find_face(frame);

        // display result
        cv::Mat scene_cross;
        frame.copyTo(scene_cross);
        for (auto center : centers) {
            draw_cross_normalized(scene_cross, center, 30);
        }
        cv::imshow("scene", scene_cross);
    } while (cv::pollKey() != 27); // Loop until ESC

    return EXIT_SUCCESS;
}

std::vector<cv::Point2f> FaceRecognizer::find_face(cv::Mat& frame) {
    cv::Point2f center(0.0f, 0.0f); 

    cv::Mat scene_grey;
    cv::cvtColor(frame, scene_grey, cv::COLOR_BGR2GRAY);

    std::vector<cv::Rect> faces;
    std::vector<cv::Point2f> centers;
    classifier.detectMultiScale(scene_grey, faces);
    if (faces.size() > 0) {
        for (auto face : faces) {
            auto x_cord = (face.x + face.width / 2.0f) / frame.cols;
            auto y_cord = (face.y + face.height / 2.0f) / frame.rows;
            cv::Point2f center(x_cord, y_cord);
            centers.push_back(center);
        }
    }

    return centers;
}

FaceRecognizer::~FaceRecognizer()
{
    cv::destroyAllWindows();
    if (captureDevice.isOpened()) {
        captureDevice.release();
    }
}