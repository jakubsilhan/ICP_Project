#include "include/runners/GLApp.hpp"


int main()
{
    GLApp glApp;

    /*if (!glApp.init_cv()) {
        return 1;
    }*/

    if (!glApp.init()) {
        return 1;
    };

    //glApp.run_cv();
    glApp.run();

    return 0;
}
