#pragma once
#include <unordered_map>
#include <string>
#include <memory>

#include "scenes/IScene.hpp"
#include "utils/Camera.hpp"

#include "render/ShaderProgram.hpp"
#include "assets/Mesh.hpp"
#include "render/Model.hpp"

class ViewerScene : public IScene {
public:
	ViewerScene(int windowWidth, int windowHeight);

	void init_assets() override;

	void process_input(GLFWwindow* window, GLfloat deltaTime) override;
	void update(float dt) override;
	void render() override;

	std::pair<double, double> get_last_cursor() override;

	void on_key(int key, int action) override;
	void on_mouse_move(double x, double y) override;
	void on_scroll(double yoffset) override;
	void on_resize(int width, int height) override;

private:
	// Camera
	Camera camera;
	double cursorLastX{ 0 };
	double cursorLastY{ 0 };

	// Assets
	std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> shader_library;
	std::unordered_map<std::string, std::shared_ptr<Mesh>> mesh_library;

	// Models
	std::unordered_map<std::string, Model> models;
	std::vector<std::string> model_names;
	int selected_model = 0;
	void next_model();
	int selected_color = 0;
	void next_color();
	void update_shader_color();

	// Transformations
	int width{ 0 };
	int height{ 0 };
	float fov = 60.0f;
	glm::mat4 projection_matrix = glm::identity<glm::mat4>();
	void update_projection_matrix();
};