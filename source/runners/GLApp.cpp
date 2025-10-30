#include "include/runners/GLApp.hpp"
#include "include/render/Triangle.hpp"
#include "include/utils/GlErrCallback.hpp"

GLApp::GLApp()
{
}

bool GLApp::init() {
    // GLFW initialization
    if (!glfwInit()) {
        std::cerr << "Error: Could not initialize GLFW.\n";
        return false;
    }
    // Version specifications
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Window creation
    window = glfwCreateWindow(800, 600, "OpenGL context", NULL, NULL);
    if (!window) {
        std::cerr << "Error: Could not create GLFW window. \n";
        glfwTerminate();
        return false;
    }

    // Context must be assigned per thread (and should not be assigned in multiple ones
    glfwMakeContextCurrent(window);

    // Requires assigned context
    if (glewInit() != GLEW_OK) {
        std::cerr << "Error: Could not initialize GLEW.\n";
        return false;
    }

    if (GLEW_ARB_debug_output) {
        glDebugMessageCallback(MessageCallback, 0);
        glEnable(GL_DEBUG_OUTPUT);

        std::cout << "GL_DEBUG enabled" << std::endl;
    }
    else {
        std::cout << "GL_DEBUG NOT SUPPORTED!" << std::endl;
    }

    // Requires initialized glew
    if (!GLEW_ARB_direct_state_access) {
        std::cerr << "Error: DSA (Direct System Access) is not available.\n";
        return false;
    }

    return true;
}

bool GLApp::run() {

    // Set background color
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Prepare triangle
    Triangle triangle;
    if (!triangle.init()) {
        std::cerr << "Error: Could not initialize triangle. \n";
        return false;
    }

    // Set triangle color
    triangle.setColor(1, 0, 0, 1);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        triangle.draw();

        // Switch background and foreground buffers (rendering is always done in background first)
        glfwSwapBuffers(window);

        // Check key, mouse events
        glfwPollEvents();

        // Sleep to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    return true;
}

GLApp::~GLApp()
{
    if (window) {
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}
