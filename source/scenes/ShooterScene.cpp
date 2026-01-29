#include <algorithm>

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
    //texture_library.emplace("yellow_flowers", std::make_shared<Texture>("resources/textures/yellow_flowers.jpg"));
    texture_library.emplace("wood_box_logos", std::make_shared<Texture>("resources/textures/wood_texture_cube_logos.png"));
    texture_library.emplace("globe", std::make_shared<Texture>("resources/textures/globe_texture.jpg"));

    // Load models
    //Model teapot_flower_model = Model("resources/meshes/teapot_tri_vnt.obj", shader_library.at("texture_shader"), texture_library.at("yellow_flowers"));
    //models.emplace("teapot_flower_object", std::move(teapot_flower_model));

    // Construct models
    Model wood_box_logos_model;
    wood_box_logos_model.addMesh(mesh_library.at("cube"), shader_library.at("texture_shader"), texture_library.at("wood_box_logos"));
    models.emplace("wood_box_logos_object", std::move(wood_box_logos_model));

    Model globe_model;
    globe_model.addMesh(mesh_library.at("sphere_highpoly"), shader_library.at("texture_shader"), texture_library.at("globe"));
    models.emplace("globe_object", std::move(globe_model));

    // Create index vector
    for (auto& [key, model] : models)
        model_names.push_back(key);

    // Load audio
    audio_manager.load("ouch", "resources/sounds/ouch.wav", 0.5f, 10000.0f, 1.0f);
    audio_manager.loadBGM("bgm", "resources/theme/03_E1M1_At_Doom's_Gate.mp3", 1.0f);

    // Play BGM
    audio_manager.playBGM("bgm", 0.2f);

    spawn_models(2, "wood_box_logos_object");
}

void ShooterScene::process_input(GLFWwindow* window, GLfloat deltaTime) {
    camera.ProcessInput(window, deltaTime);
    camera.Position = clamp_to_bounds(camera.Position, world_bounds);
}

void ShooterScene::update(float dt) {

}

void ShooterScene::render() {

    // Update listener location and clear sounds
    audio_manager.setListenerPosition(camera.Position.x, camera.Position.y, camera.Position.z, camera.Front.x, camera.Front.y, camera.Front.z);
    audio_manager.cleanFinishedSounds();

    // Model selection
    /*Model& model = models[model_names[selected_model]];
    model.draw(camera.GetViewMatrix(), projection_matrix);*/

    for (auto& sm : spawned_models) {
        if (!sm.active) continue;

        auto& model = models[sm.model_name];
        model.setPosition(sm.position); // update model's transform
        model.draw(camera.GetViewMatrix(), projection_matrix);
    }

    // All models
    /*for (auto& [name, model] : models) {
        model.draw();
    }*/
}

#pragma region targets
void ShooterScene::spawn_models(int count, const std::string& model_name) {
    for (int i = 0; i < count; ++i) {
        SpawnedModel m;
        m.model_name = model_name;
        m.position = random_position_in_bounds();
        m.respawn_time = default_respawn_time;
        m.timer = 0.0f;
        m.active = true;
        spawned_models.push_back(m);
    }
}

#pragma endregion

#pragma region Utils
void ShooterScene::next_model() {
    selected_model = (selected_model + 1) % model_names.size();
}

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
        next_model();
        break;
    case GLFW_KEY_H:
        auto m_pos = models[model_names[selected_model]].getPosition();
        audio_manager.play3D(
            "ouch",           // name
            m_pos.x, m_pos.y, m_pos.z // Sound Source Position
        );
        break;
    default:
        break;
    }
}

void ShooterScene::on_mouse_move(double x, double y) {
    camera.ProcessMouseMovement(x - cursorLastX, (y - cursorLastY) * -1.0);
    cursorLastX = x;
    cursorLastY = y;
}

void ShooterScene::on_scroll(double offset) {
    fov -= 10 * offset;
    fov = std::clamp(fov, 10.0f, 170.0f); // limit FOV to reasonable values

    update_projection_matrix();
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