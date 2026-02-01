#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class IScene {
public:
    virtual ~IScene() = default;

    virtual void init_assets() = 0;

    virtual void set_enabled(bool enabled) = 0;
    virtual void process_input(GLFWwindow* window, GLfloat deltaTime) = 0;
    virtual void update(float dt) = 0;
    virtual void render() = 0;
    virtual void display_controls() = 0;

    virtual std::pair<double, double> get_last_cursor() = 0;

    virtual void on_mouse_button(int button, int action) = 0;
    virtual void on_key(int key, int action) = 0;
    virtual void on_mouse_move(double x, double y) = 0;
    virtual void on_resize(int width, int height) = 0;
    virtual void on_scroll(double yoffset) = 0;
};
