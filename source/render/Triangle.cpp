#include "render/Triangle.hpp"
#include "render/Model.hpp"

#define TRIANGLE_OBJ_PATH "resources/triangle.obj"

Triangle::Triangle(std::shared_ptr<ShaderProgram> shader) :
    Model{std::filesystem::path(TRIANGLE_OBJ_PATH), shader}, shader{shader} {}

void Triangle::draw() {
    shader->setUniform(std::string("useUniformColor"), true);
    shader->setUniform(std::string("uniformColor"), color);
    Model::draw();
}

void Triangle::setColor(float r, float g, float b, float a) {
    color = glm::vec4(r, g, b, a);
}
