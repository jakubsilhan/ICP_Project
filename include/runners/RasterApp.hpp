#pragma once

#include <vector>

#include <opencv2/opencv.hpp>

class RasterApp {
public:
    RasterApp();

    bool init(void);

    std::vector<uchar> lossy_bw_limit(cv::Mat& input_img, size_t size_limit);
    std::vector<uchar> lossy_quality_limit(const cv::Mat& frame, const float target_coefficient);

    int run(void);

    ~RasterApp();
private:
    cv::VideoCapture capture;
    bool bandwidth=false;
};
