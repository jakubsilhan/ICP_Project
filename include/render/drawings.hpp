#pragma once
#include <opencv2/opencv.hpp>

void draw_cross_normalized(cv::Mat& img, cv::Point2f center_relative, int size);
void draw_cross(cv::Mat& img, int x, int y, int size);