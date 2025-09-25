#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>

class App {
public:
    App(); // default constructor, called on app instance definition

    // public methods
    bool init(void);
    int run(void);

    void draw_cross_normalized(cv::Mat& img, cv::Point2f center_relative, int size);
    void draw_cross(cv::Mat& img, int x, int y, int size);

    ~App(); //default destructor, called on app instance destruction
private:
    // no private variables, so far...
};