#include "include/runners/GLApp.hpp"


int main()
{
    GLApp glApp;

    if (!glApp.init()) {
        return 1;
    };

    glApp.run();

    return 0;
}
