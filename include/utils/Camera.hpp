#pragma once

#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:

    // Camera Attributes
    glm::vec3 position{};
    glm::vec3 front{};
    glm::vec3 right{};
    glm::vec3 up{}; // camera local UP vector

    GLfloat yaw = -90.0f;
    GLfloat pitch = 0.0f;
    GLfloat roll = 0.0f;

    // Camera options
    GLfloat movement_speed = 5.0f;
    GLfloat mouse_sensitivity = 0.10f;

    Camera(){
        // Default constructor initializes camera's position and orientation
        this->update_camera_vectors();
    }

    Camera(glm::vec3 position) :position(position)
    {
        this->up = glm::vec3(0.0f, 1.0f, 0.0f);
        // initialization of the camera reference system
        this->update_camera_vectors();
    }

    glm::mat4 get_view_matrix()
    {
        return glm::lookAt(this->position, this->position + this->front, this->up);
    }

    void reset(glm::vec3 position) {
        yaw = -90.0f;
        pitch = 0.0f;
        roll = 0.0f;
        this->position = position;
        this->update_camera_vectors();
    }

    void process_input(GLFWwindow* window, GLfloat deltaTime)
    {
        glm::vec3 direction{ 0 };
        GLfloat multiplier = 1.0f;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            direction += front;

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            direction -= front;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            direction -= right;

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            direction += right;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            direction += up;
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
            direction -= up;
        //... up, down, diagonal, ... 
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            multiplier = 2.0f;

        if (glm::length(direction) > 0.0001f)
            position += glm::normalize(direction) * movement_speed * multiplier * deltaTime;
    }

    void process_mouse_movement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch = GL_TRUE)
    {
        xoffset *= this->mouse_sensitivity;
        yoffset *= this->mouse_sensitivity;

        this->yaw += xoffset;
        this->pitch += yoffset;

        if (constraintPitch)
        {
            if (this->pitch > 89.0f)
                this->pitch = 89.0f;
            if (this->pitch < -89.0f)
                this->pitch = -89.0f;
        }

        this->update_camera_vectors();
    }

private:
    void update_camera_vectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
        front.y = sin(glm::radians(this->pitch));
        front.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));

        this->front = glm::normalize(front);
        this->right = glm::normalize(glm::cross(this->front, glm::vec3(0.0f, 1.0f, 0.0f)));
        this->up = glm::normalize(glm::cross(this->right, this->front));
    }
};
