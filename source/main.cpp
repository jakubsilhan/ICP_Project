#include "include/runners/GLApp.hpp"
#include "include/runners/TrackApp.hpp"
#include "include/runners/ThreadTrackApp.hpp"
#include "include/scenes/ShooterScene.hpp"
#define MINIAUDIO_IMPLEMENTATION
#include "audio/Miniaudio.h"

int main()
{

    #ifdef RUN_GLAPP
        GLApp glApp;
        if (glApp.init()) glApp.run();
    #endif

    #ifdef RUN_TRACKAPP
        TrackApp trackApp;
        if (trackApp.init()) trackApp.run();
    #endif

    #ifdef RUN_THREADTRACKAPP
        ThreadTrackApp threadTrackApp;
        if (threadTrackApp.init()) threadTrackApp.run();
    #endif

    return 0;
}
