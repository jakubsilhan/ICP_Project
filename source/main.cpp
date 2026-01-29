#include "include/runners/RasterApp.hpp"
#include "include/runners/GLApp.hpp"
#define MINIAUDIO_IMPLEMENTATION
#include "audio/Miniaudio.h"


int main()
{
    // Raster processing App
    /*RasterApp rastApp;
    if (!rastApp.init()) {
        return 1;
    }
    rastApp.run();*/

    // Main App
    GLApp glApp;
    if (!glApp.init()) {
        return 1;
    };
    glApp.run();

    return 0;
}
