#include <memory>
#include <filesystem>

#include <glm/glm.hpp>

#include "render/Triangle.hpp"
#include "render/Model.hpp"

#define TRIANGLE_OBJ_PATH "resources/meshes/triangle.obj"

Triangle::Triangle(std::shared_ptr<ShaderProgram> shader) :
    Model{std::filesystem::path(TRIANGLE_OBJ_PATH), shader}, shader{shader} {}

void Triangle::draw(const glm::mat4& view_matrix, const glm::mat4& projection_matrix) {
    shader->set_uniform(std::string("useUniformColor"), true);
    shader->set_uniform(std::string("uniformColor"), color);
    Model::draw(view_matrix, projection_matrix);
}

void Triangle::set_color(float r, float g, float b, float a) {
    color = glm::vec4(r, g, b, a);
}
