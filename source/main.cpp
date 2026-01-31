#include "include/runners/GLApp.hpp"
#include "include/scenes/ShooterScene.hpp"
#define MINIAUDIO_IMPLEMENTATION
#include "audio/Miniaudio.h"

int main()
{
    GLApp glApp;
    if (glApp.init()) glApp.run();

    return 0;
}
