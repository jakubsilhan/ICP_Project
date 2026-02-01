#include <vector>
#include <numeric>
#include <chrono>
#include <iostream>

#include <opencv2/opencv.hpp>

#include "recognizers/RedRecognizer.hpp"
#include "render/Drawings.hpp"

RedRecognizer::RedRecognizer() {
    // Constructor
}

int RedRecognizer::run_static(std::string path) {
    cv::Mat frame = cv::imread(path);

    if (frame.empty()) {
        std::cerr << "Error: Could not load the static image." << std::endl;
        return EXIT_FAILURE;
    }

    cv::Mat scene;

    // Safekeep original frame
    frame.copyTo(scene);
    cv::imshow("grabbed", frame);

    // Analyze and draw
    cv::Point2f center = RedRecognizer::find_red(scene);
    draw_cross_normalized(scene, center, 30);
    cv::imshow("analyzed", scene);

    // Pause until a key is pressed
    cv::waitKey(0);

    // Clean up windows
    cv::destroyAllWindows();

    // Return success status
    return EXIT_SUCCESS;
}

int RedRecognizer::run_video(std::string path) {
    // Initialization
    capture_device = cv::VideoCapture(path);

    if (!capture_device.isOpened()) {
        std::cerr << "no camera" << std::endl;
        exit(EXIT_FAILURE);
    }

    cv::Mat frame, scene;

    // Watch video
    while (1) {
        // Check video
        capture_device.read(frame);
        if (frame.empty()) {
            std::cerr << "No video" << std::endl;
            break;
        }

        // Safekeep original frames
        frame.copyTo(scene);
        cv::imshow("grabbed", frame);

        // Analyze and draw
        cv::Point2f center = RedRecognizer::find_red(scene);
        draw_cross_normalized(scene, center, 30);
        cv::imshow("analyzed", scene);

        // Close player
        if (cv::waitKey(1) == 27) {
            break;
        }
    }

    if (capture_device.isOpened()) {
        capture_device.release();
    }
    cv::destroyAllWindows();
    exit(EXIT_SUCCESS);
    
}

cv::Point2f RedRecognizer::find_red(cv::Mat& frame) {
    auto start = std::chrono::steady_clock::now();
    cv::Mat img_HSV, mask_low_red, mask_high_red, mask_combined;

    // Convert image to HSV
    cv::cvtColor(frame, img_HSV, cv::COLOR_BGR2HSV);

    // Prepare red hue ranges
    cv::Scalar lower_red1 = cv::Scalar(0, 150, 150); // Hue, Saturation, Value (Brightness)
    cv::Scalar upper_red1 = cv::Scalar(20, 255, 255);
    cv::Scalar lower_red2 = cv::Scalar(160, 150, 150);
    cv::Scalar upper_red2 = cv::Scalar(180, 255, 255);

    // Checks within range
    cv::inRange(img_HSV, lower_red1, upper_red1, mask_low_red);
    cv::inRange(img_HSV, lower_red2, upper_red2, mask_high_red);

    // Combines masks
    cv::bitwise_or(mask_low_red, mask_high_red, mask_combined);

    // Morphological Operations for noise reduction (TODO)

    // Finds masked pixels
    std::vector<cv::Point> white_pixels;
    cv::findNonZero(mask_combined, white_pixels);

    // If no red pixels are found, return the default (0.0f, 0.0f)
    if (white_pixels.size() == 0) {
        return cv::Point2f(0.0f, 0.0f);
    }

    // Calculate Centroid
    cv::Point2f white_accum = std::reduce(white_pixels.begin(), white_pixels.end()); // Sums x and y coords

    cv::Point2f centroid_absolute = white_accum / (float)white_pixels.size(); // Gets centroid

    cv::Point2f centroid_normalized = { // Normalize
        centroid_absolute.x / frame.cols,
        centroid_absolute.y / frame.rows
    };
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end - start;
    std::cout << "Elapsed time: " << elapsed_seconds.count()*1000 << " millisec" << std::endl;

    return centroid_normalized;
}
