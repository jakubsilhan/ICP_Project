// Project.cpp : Defines the entry point for the application.
//

#include "Project.h"
#include <opencv2/opencv.hpp>
#include <iostream>

int runProject() {
    // Create a black image of size 400x400
    cv::Mat image = cv::Mat::zeros(400, 400, CV_8UC3);

    // Draw a green circle in the center
    cv::circle(image, cv::Point(200, 200), 100, cv::Scalar(0, 255, 0), -1);

    // Draw a red rectangle
    cv::rectangle(image, cv::Point(50, 50), cv::Point(350, 350), cv::Scalar(0, 0, 255), 3);

    // Show the image in a window
    cv::imshow("Test Window", image);

    std::cout << "Press any key to exit..." << std::endl;
    cv::waitKey(0);  // Wait for key press

    cv::destroyAllWindows();
    return 0;
}
