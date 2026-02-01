#pragma once

#include <opencv2/opencv.hpp>

void draw_cross_normalized(cv::Mat& img, cv::Point2f center_relative, int size, const cv::Scalar& color = CV_RGB(255, 0, 0));
void draw_cross(cv::Mat& img, int x, int y, int size, const cv::Scalar& color = CV_RGB(255, 0, 0));
