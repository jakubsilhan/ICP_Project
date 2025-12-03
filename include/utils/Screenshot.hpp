#pragma once

#include <filesystem>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <FreeImage.h>

bool makeScreenshot(const char * path, const GLint x, const GLint y, const GLsizei width, const GLsizei height) {

    // Source - https://stackoverflow.com/a/6938633
    // Posted by huy, modified by community. See post 'Timeline' for change history
    // Retrieved 2025-12-01, License - CC BY-SA 3.0
    // Modified for use in this project

    // Make the GLubyte array, factor of 3 because it's RBG.
    GLubyte* pixels = new GLubyte[3 * width * height];

    glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixels);

    // Convert to FreeImage format
    FIBITMAP* image = FreeImage_ConvertFromRawBits(pixels, width, height, 3 * width, 24, 0, 0, 0, false);
    if (!image) {
        return false;
    }

    // Save to file
    if (!FreeImage_Save(FIF_PNG, image, path, 0)) {
        return false;
    }

    // Free resources
    FreeImage_Unload(image);
    delete [] pixels;

    return true;
}
