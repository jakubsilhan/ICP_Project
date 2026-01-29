#pragma once
#include <unordered_map>
#include <string>
#include <memory>

#include "scenes/IScene.hpp"
#include "utils/Camera.hpp"

#include "audio/AudioManager.hpp"
#include "render/ShaderProgram.hpp"
#include "assets/Mesh.hpp"
#include "render/Model.hpp"
#include "render/Texture.hpp"

//struct AABB {
//	glm::vec3 min;
//	glm::vec3 max;
//
//	// Check if a point is inside the AABB
//	bool contains(const glm::vec3& point) const {
//		return point.x >= min.x && point.x <= max.x &&
//			point.y >= min.y && point.y <= max.y &&
//			point.z >= min.z && point.z <= max.z;
//	}
//
//	// Center of AABB
//	glm::vec3 center() const {
//		return (min + max) * 0.5f;
//	}
//
//	// Get half-extents of AABB
//	glm::vec3 halfExtents() const {
//		return (max - min) * 0.5f;
//	}
//};



struct Target {
	Model* model = nullptr;
	glm::vec3 position;
	float respawn_time;   // seconds until it can respawn
	float timer = 0.0f;  // current countdown
	bool active = true;   // currently spawned or "dead"
	float scale = 1.0f;   // for bounding box calculation
	glm::vec3 velocity;
	float maxSpeed = 5.0f;

	AABB getBoundingBox() const {
		AABB local = model->getLocalAABB();

		glm::vec3 min = local.min * scale + position;
		glm::vec3 max = local.max * scale + position;

		return { min, max };
	}
};

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;

	glm::vec3 pointAt(float t) const {
		return origin + direction * t;
	}
};

struct RayHit {
	bool hit = false;
	float distance = 0.0f;
	glm::vec3 point;
	int modelIndex = -1;
};

class ShooterScene : public IScene {
public:
	ShooterScene(int windowWidth, int windowHeight);
	void init_assets() override;
	void process_input(GLFWwindow* window, GLfloat deltaTime) override;

	void update(float dt) override;
	void render() override;

	std::pair<double, double> get_last_cursor() override;

	void on_mouse_button(int button, int action);
	void on_key(int key, int action) override;
	void on_mouse_move(double x, double y) override;
	void on_scroll(double yoffset) override;
	void on_resize(int width, int height) override;

private:
	// Camera
	Camera camera;
	double cursorLastX{ 0 };
	double cursorLastY{ 0 };

	// Audio
	AudioManager audio_manager;

	// Assets
	std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> shader_library;
	std::unordered_map<std::string, std::shared_ptr<Mesh>> mesh_library;
	std::unordered_map<std::string, std::shared_ptr<Texture>> texture_library;

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

	// Targets
	std::vector<Target> spawned_models;
	void spawn_models(int count, const std::string& model_name);
	float default_respawn_time = 5.0f;

	// Shooting mechanics
	Ray create_ray_from_camera();
	RayHit raycast(const Ray& ray);
	bool ray_aabb_intersection(const Ray& ray, const AABB& aabb, float& t);
	void shoot();

	// Bounds
	AABB world_bounds{
		glm::vec3(-25.0f, -10.0f, -25.0f),
		glm::vec3(25.0f,  10.0f,  25.0f)
	};
	glm::vec3 clamp_to_bounds(const glm::vec3& p, const AABB& b);
	glm::vec3 random_position_in_bounds();
};