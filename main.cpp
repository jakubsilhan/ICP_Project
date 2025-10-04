#include "include/runners/TrackApp.hpp"


int main()
{
    TrackApp trackApp;

    if (!trackApp.init()) {
        return 1;
    };

    trackApp.run();

    return 0;
}
