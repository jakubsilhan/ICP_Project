#include "include/runners/ThreadTrackApp.hpp"


int main()
{
    ThreadTrackApp trackApp;

    if (!trackApp.init()) {
        return 1;
    };

    trackApp.run();

    return 0;
}
