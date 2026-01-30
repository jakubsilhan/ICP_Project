#include "include/runners/RasterApp.hpp"
#include "include/runners/GLApp.hpp"
#include "include/scenes/ShooterScene.hpp"
#define MINIAUDIO_IMPLEMENTATION
#include "audio/Miniaudio.h"


int main()
{
    std::cout << "==============================" << std::endl;
    std::cout << "   APPLICATION SELECTOR       " << std::endl;
    std::cout << "==============================" << std::endl;
    std::cout << "1 - Raster Processing App" << std::endl;
    std::cout << "2 - Main OpenGL App" << std::endl;
    std::cout << "Selection: ";

    char selection;
    std::cin >> selection;

    switch (selection) {
    case '1': {
        RasterApp rastApp;
        if (rastApp.init()) rastApp.run();
        break;
    }
    case '2': {
        GLApp glApp;
        if (glApp.init()) glApp.run();
        break;
    }
    default:
        std::cout << "Exiting!" << std::endl;
        break;
    }

    return 0;
}
