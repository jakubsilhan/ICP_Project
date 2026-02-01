#pragma once

#include <GL/glew.h>
#include <opencv2/opencv.hpp>

#include "utils/PathUtils.hpp"

bool make_screenshot(const char * path, const GLint x, const GLint y, const GLsizei width, const GLsizei height) {

    if (!ends_with_ext(path, ".png")) {
        return false;
    }

    // Make the GLubyte array
    std::vector<GLubyte> pixels(width * height * 3); // 3 for RGB

    glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixels.data());

    // Convert to cv::Mat
    cv::Mat image(height, width, CV_8UC3, pixels.data());

    // Flip vertically because OpenGL's origin is bottom-left
    cv::flip(image, image, 0);

    // Save to file
    if (!cv::imwrite(path, image)) {
        return false;
    }

    return true;
}
