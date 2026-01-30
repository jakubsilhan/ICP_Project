#include <algorithm>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "scenes/ShooterScene.hpp"
#include "render/Model.hpp"
#include "utils/Camera.hpp"
#include "utils/MeshGen.hpp"

ShooterScene::ShooterScene(int windowWidth, int windowHeight) {
    width = windowWidth;
    height = windowHeight;
    camera.Position = glm::vec3(0, 0, 10);
    update_projection_matrix();

    init_assets();
    update_shader_color();
}

void ShooterScene::init_assets() {
    // Load shaders
    shader_library.emplace("simple_shader", std::make_shared<ShaderProgram>(std::filesystem::path("resources/basic_sdr/basic.vert"), std::filesystem::path("resources/basic_sdr/basic.frag")));
    shader_library.emplace("texture_shader", std::make_shared<ShaderProgram>(std::filesystem::path("resources/texture_sdr/tex.vert"), std::filesystem::path("resources/texture_sdr/tex.frag")));
    shader_library.at("texture_shader")->setUniform("tex0", 0);

    // Load meshes
    mesh_library.emplace("cube", generate_cube(cube_atlas_cross));
    mesh_library.emplace("sphere_highpoly", generate_sphere(8, 8));

    // Load textures
    texture_library.emplace("yellow_flowers", std::make_shared<Texture>("resources/textures/yellow_flowers.jpg"));
    texture_library.emplace("wood_box", std::make_shared<Texture>("resources/textures/box_rgb888.png"));
    texture_library.emplace("wood_box_logos", std::make_shared<Texture>("resources/textures/wood_texture_cube_logos.png"));
    texture_library.emplace("globe", std::make_shared<Texture>("resources/textures/globe_texture.jpg"));
    //texture_library.emplace("trex", std::make_shared<Texture>("resources/textures/trex_diff.png"));

    // Load models
    Model teapot_flower_model = Model("resources/meshes/teapot_tri_vnt.obj", shader_library.at("texture_shader"), texture_library.at("yellow_flowers"));
    models.emplace("teapot_flower_object", std::move(teapot_flower_model));
    Model bunny_model = Model("resources/meshes/bunny_tri_vnt.obj", shader_library.at("simple_shader"));
    models.emplace("bunny_object", std::move(bunny_model));
    /*Model trex_model = Model("resources/meshes/trex.obj", shader_library.at("texture_shader"), texture_library.at("trex"));
    models.emplace("trex_object", std::move(trex_model));*/

    // Construct models
    Model wood_box_logos_model;
    wood_box_logos_model.addMesh(mesh_library.at("cube"), shader_library.at("texture_shader"), texture_library.at("wood_box_logos"));
    models.emplace("wood_box_logos_object", std::move(wood_box_logos_model));

    Model wood_box_model;
    wood_box_logos_model.addMesh(mesh_library.at("cube"), shader_library.at("texture_shader"), texture_library.at("wood_box"));
    models.emplace("wood_box_object", std::move(wood_box_model));

    Model globe_model;
    globe_model.addMesh(mesh_library.at("sphere_highpoly"), shader_library.at("texture_shader"), texture_library.at("globe"));
    models.emplace("globe_object", std::move(globe_model));

    // Create index vector
    for (auto& [key, model] : models)
        model_names.push_back(key);

    // Load audio
    audio_manager.load("ping", "resources/sounds/ouch.wav", 0.5f, 10000.0f, 1.0f);
    audio_manager.load("shot", "resources/sounds/step1.wav", 0.5f, 10000.0f, 1.0f);
    audio_manager.loadBGM("bgm", "resources/theme/03_E1M1_At_Doom's_Gate.mp3", 1.0f);

    //// Play BGM
    audio_manager.playBGM("bgm", 0.2f);

    spawn_models(1, "wood_box_logos_object");
    spawn_models(1, "wood_box_object");
    spawn_models(1, "globe_object");
    spawn_models(1, "teapot_flower_object");
    spawn_models(1, "bunny_object");
    //spawn_models(1, "trex_object");
}

void ShooterScene::process_input(GLFWwindow* window, GLfloat deltaTime) {
    camera.ProcessInput(window, deltaTime);
    camera.Position = clamp_to_bounds(camera.Position, world_bounds);
}

void ShooterScene::update(float dt) {
    // respawn
    for (auto& sm : spawned_models) {
        if (!sm.active) {
            sm.timer += dt;
            if (sm.timer >= sm.respawn_time) {
                sm.position = random_position_in_bounds();
                sm.timer = 0.0f;
                sm.scale = 1.0f;
                sm.active = true;

                float speed = ((float)rand() / RAND_MAX) * 1.5f + 0.5f;
                float angleXY = ((float)rand() / RAND_MAX) * 2.0f * glm::pi<float>();
                float angleZ = ((float)rand() / RAND_MAX) * 2.0f * glm::pi<float>();
                sm.velocity = glm::vec3(
                    cos(angleXY) * cos(angleZ),
                    sin(angleXY) * cos(angleZ),
                    sin(angleZ)
                ) * speed;
            }
            continue;
        }
        // Move the active target
        sm.position += sm.velocity * dt;

        // Bounce off world bounds
        if (sm.position.x < world_bounds.min.x || sm.position.x > world_bounds.max.x) sm.velocity.x *= -1.0f;
        if (sm.position.y < world_bounds.min.y || sm.position.y > world_bounds.max.y) sm.velocity.y *= -1.0f;
        if (sm.position.z < world_bounds.min.z || sm.position.z > world_bounds.max.z) sm.velocity.z *= -1.0f;
    }
}

void ShooterScene::render() {

    // Update listener location and clear sounds
    audio_manager.setListenerPosition(camera.Position.x, camera.Position.y, camera.Position.z, camera.Front.x, camera.Front.y, camera.Front.z);
    audio_manager.cleanFinishedSounds();

    for (auto& sm : spawned_models) {
        if (!sm.active) continue;

        sm.model->setPosition(sm.position); // update model's transform
        sm.model->draw(camera.GetViewMatrix(), projection_matrix);
    }
}

void ShooterScene::display_controls() {
    ImGui::Text("Controls:");
    ImGui::Text("X - Reset camera");
    ImGui::Text("E - switch color");
    ImGui::Text("Q - ping next target");
    ImGui::Text("Scroll - change bgm volume");
    ImGui::Text("Movement:");
    ImGui::Text("Left Click - Enter Movement Mode / Shoot");
    ImGui::Text("Right Click - Exit Movement Mode");
    ImGui::Text("WASD + Space + C - Movement");
    ImGui::Text("Left Shift - Speed Boost");
}

#pragma region Targets
void ShooterScene::spawn_models(int count, const std::string& model_name) {
    Model& model = models.at(model_name);
    for (int i = 0; i < count; ++i) {
        Target t;
        t.model = &model;
        t.position = random_position_in_bounds();
        t.respawn_time = default_respawn_time;
        t.timer = 0.0f;
        t.scale = 1.0f;
        t.active = true;

        // Random initial velocity
        float speed = ((float)rand() / RAND_MAX) * (t.maxSpeed-0.5f) + 0.5f;
        float angleXY = ((float)rand() / RAND_MAX) * 2.0f * glm::pi<float>();
        float angleZ = ((float)rand() / RAND_MAX) * 2.0f * glm::pi<float>();
        t.velocity = glm::vec3(
            cos(angleXY) * cos(angleZ),
            sin(angleXY) * cos(angleZ),
            sin(angleZ)
        ) * speed;

        spawned_models.push_back(t);
    }
}

#pragma endregion

#pragma region shooting
Ray ShooterScene::create_ray_from_camera() {
    Ray ray;
    ray.origin = camera.Position;
    ray.direction = glm::normalize(camera.Front);
    return ray;
}

bool ShooterScene::ray_aabb_intersection(const Ray& ray, const AABB& aabb, float& t) {
    // Slab method for ray-AABB intersection
    glm::vec3 invDir = 1.0f / ray.direction;

    glm::vec3 t0 = (aabb.min - ray.origin) * invDir;
    glm::vec3 t1 = (aabb.max - ray.origin) * invDir;

    glm::vec3 tmin = glm::min(t0, t1);
    glm::vec3 tmax = glm::max(t0, t1);

    float tNear = glm::max(glm::max(tmin.x, tmin.y), tmin.z);
    float tFar = glm::min(glm::min(tmax.x, tmax.y), tmax.z);

    if (tNear > tFar || tFar < 0.0f) {
        return false; // No intersection
    }

    t = (tNear < 0.0f) ? tFar : tNear;
    return true;
}

RayHit ShooterScene::raycast(const Ray& ray) {
    RayHit result;
    result.hit = false;
    result.distance = std::numeric_limits<float>::max();

    for (size_t i = 0; i < spawned_models.size(); ++i) {
        if (!spawned_models[i].active) continue;

        AABB bbox = spawned_models[i].getBoundingBox();
        float t;

        if (ray_aabb_intersection(ray, bbox, t)) {
            if (t < result.distance) {
                result.hit = true;
                result.distance = t;
                result.point = ray.pointAt(t);
                result.modelIndex = static_cast<int>(i);
            }
        }
    }

    return result;
}

void ShooterScene::shoot() {
    Ray ray = create_ray_from_camera();
    RayHit hit = raycast(ray);
    // Play shooting sound
    audio_manager.play3D("shot", camera.Position.x, camera.Position.y, camera.Position.z);

    if (hit.hit && hit.modelIndex >= 0) {
        // Hit a target
        Target& target = spawned_models[hit.modelIndex];

        // Deactivate the target
        target.active = false;
        target.timer = 0.0f;
    }
}
#pragma endregion

#pragma region Utils
std::pair<double, double> ShooterScene::get_last_cursor() {
    return { cursorLastX, cursorLastY };
}

void ShooterScene::next_color() {
    selected_color = (selected_color + 1) % 3;

    update_shader_color();
}

void ShooterScene::update_shader_color() {
    glm::vec4 shader_color;
    switch (selected_color) {
    case 0: shader_color = glm::vec4(1, 0, 0, 1); break;
    case 1: shader_color = glm::vec4(0, 1, 0, 1); break;
    case 2: shader_color = glm::vec4(0, 0, 1, 1); break;
    }
    shader_library.at("simple_shader")->setUniform("uniformColor", shader_color);
}

glm::vec3 ShooterScene::clamp_to_bounds(const glm::vec3& p, const AABB& b) {
    return glm::clamp(p, b.min, b.max);
}

glm::vec3 ShooterScene::random_position_in_bounds() {
    float x = world_bounds.min.x + static_cast<float>(rand()) / RAND_MAX * (world_bounds.max.x - world_bounds.min.x);
    float y = world_bounds.min.y + static_cast<float>(rand()) / RAND_MAX * (world_bounds.max.y - world_bounds.min.y);
    float z = world_bounds.min.z + static_cast<float>(rand()) / RAND_MAX * (world_bounds.max.z - world_bounds.min.z);
    return glm::vec3(x, y, z);
}
#pragma endregion

#pragma region Listeners
void ShooterScene::on_key(int key, int action) {
    if (action != GLFW_PRESS) return;
    switch (key) {
    case GLFW_KEY_E:
        next_color();
        break;
    case GLFW_KEY_X:
        camera.Reset(glm::vec3(0, 0, 10));
        break;
    case GLFW_KEY_Q:
        for(auto& sm : spawned_models) {
            if (sm.active) {
                glm::vec3 target_pos = sm.position;
                audio_manager.play3D("ping", target_pos.x, target_pos.y, target_pos.z);
            }
        }
        break;
    default:
        break;
    }
}

void ShooterScene::on_mouse_button(int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        shoot();
    }
}

void ShooterScene::on_mouse_move(double x, double y) {
    camera.ProcessMouseMovement(x - cursorLastX, (y - cursorLastY) * -1.0);
    cursorLastX = x;
    cursorLastY = y;
}

void ShooterScene::on_scroll(double offset) {
    audio_manager.changeVolume(offset);
}

void ShooterScene::on_resize(int w, int h) {
    width = w;
    height = h;

    update_projection_matrix();
}
#pragma endregion

#pragma region Transformation
void ShooterScene::update_projection_matrix()
{
    if (height < 1)
        height = 1;   // avoid division by 0

    float ratio = static_cast<float>(width) / height;

    projection_matrix = glm::perspective(
        glm::radians(fov),   // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90 (extra wide) and 30 (quite zoomed in)
        ratio,               // Aspect Ratio. Depends on the size of your window.
        0.1f,                // Near clipping plane. Keep as big as possible, or you'll get precision issues.
        20000.0f             // Far clipping plane. Keep as little as possible.
    );
}
#pragma endregion
