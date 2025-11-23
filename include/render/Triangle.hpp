#pragma once

#include "render/Model.hpp"

class Triangle : private Model {
    public:
        Triangle(std::shared_ptr<ShaderProgram> shader);
        void draw();
        void setColor(float r, float g, float b, float a);
    private:
        std::shared_ptr<ShaderProgram> shader;
        glm::vec4 color;
};
