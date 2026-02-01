#include <algorithm>

#include <glm/glm.hpp>

// ImGUI
#include <imgui.h>               // main ImGUI header
#include <imgui_impl_glfw.h>     // GLFW bindings
#include <imgui_impl_opengl3.h>  // OpenGL bindings

#include "scenes/ViewerScene.hpp"
#include "render/Model.hpp"
#include "utils/Camera.hpp"
#include "utils/MeshGen.hpp"

ViewerScene::ViewerScene(int window_width, int window_height) {
    width = window_width;
    height = window_height;
    camera.position = glm::vec3(0, 0, 10);
    update_projection_matrix();
    //projection_matrix = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 1000.0f);

    init_assets();
    update_shader_color();
}

void ViewerScene::init_assets() {
    // Load shaders
    shader_library.emplace("simple_shader", std::make_shared<ShaderProgram>(std::filesystem::path("resources/basic_sdr/basic.vert"), std::filesystem::path("resources/basic_sdr/basic.frag")));
    shader_library.emplace("texture_shader", std::make_shared<ShaderProgram>(std::filesystem::path("resources/texture_sdr/tex.vert"), std::filesystem::path("resources/texture_sdr/tex.frag")));
    shader_library.at("texture_shader")->set_uniform("tex0", 0);

    // Load meshes
    mesh_library.emplace("cube_single", generate_cube(cube_atlas_single));
    mesh_library.emplace("cube", generate_cube(cube_atlas_cross));
    mesh_library.emplace("sphere_lowpoly", generate_sphere(4, 4));
    mesh_library.emplace("sphere_highpoly", generate_sphere(8, 8));

    // Load textures
    texture_library.emplace("yellow_flowers", std::make_shared<Texture>("resources/textures/yellow_flowers.jpg"));
    texture_library.emplace("wood_box", std::make_shared<Texture>("resources/textures/box_rgb888.png"));
    texture_library.emplace("wood_box_logos", std::make_shared<Texture>("resources/textures/wood_texture_cube_logos.png"));
    texture_library.emplace("globe", std::make_shared<Texture>("resources/textures/globe_texture.jpg"));

    // Load models
    Model teapot_model = Model("resources/meshes/teapot_tri_vnt.obj", shader_library.at("simple_shader"));
    Model teapot_flower_model = Model("resources/meshes/teapot_tri_vnt.obj", shader_library.at("texture_shader"), texture_library.at("yellow_flowers"));
    models.emplace("teapot_object", std::move(teapot_model));
    models.emplace("teapot_flower_object", std::move(teapot_flower_model));

    // Construct models
    Model cube_model;
    cube_model.add_mesh(mesh_library.at("cube_single"), shader_library.at("simple_shader"));
    models.emplace("cube_object", std::move(cube_model));

    Model wood_box_model;
    wood_box_model.add_mesh(mesh_library.at("cube_single"), shader_library.at("texture_shader"), texture_library.at("wood_box"));
    models.emplace("wood_box_object", std::move(wood_box_model));

    Model wood_box_logos_model;
    wood_box_logos_model.add_mesh(mesh_library.at("cube"), shader_library.at("texture_shader"), texture_library.at("wood_box_logos"));
    models.emplace("wood_box_logos_object", std::move(wood_box_logos_model));

    Model sphere_l_model;
    sphere_l_model.add_mesh(mesh_library.at("sphere_lowpoly"), shader_library.at("simple_shader"));
    models.emplace("sphere_l_object", std::move(sphere_l_model));

    Model sphere_h_model;
    sphere_h_model.add_mesh(mesh_library.at("sphere_highpoly"), shader_library.at("simple_shader"));
    models.emplace("sphere_h_object", std::move(sphere_h_model));

    Model globe_model;
    globe_model.add_mesh(mesh_library.at("sphere_highpoly"), shader_library.at("texture_shader"), texture_library.at("globe"));
    models.emplace("globe_object", std::move(globe_model));

    // Create index vector
    for (auto& [key, model] : models)
        model_names.push_back(key);

    // Load audio
    audio_manager.load("ping", "resources/sounds/ping.wav", 0.5f, 10000.0f, 1.0f);
    audio_manager.load_BGM("bgm", "resources/theme/03_E1M1_At_Doom's_Gate.mp3", 1.0f);
    //audio_manager.load("step1", "resources/sounds/step1.wav");
    //audio_manager.load("step2", "resources/sounds/step2.wav");

    // Play BGM
    audio_manager.play_BGM("bgm", 0.2f);
}

void ViewerScene::set_enabled(bool enabled) {
    this->enabled = enabled;
}

void ViewerScene::process_input(GLFWwindow* window, GLfloat deltaTime) {
    if (!this->enabled) return;

    camera.process_input(window, deltaTime);
}

void ViewerScene::update(float dt) {
    
}

void ViewerScene::render() {
    // Update listener location and clear sounds
    audio_manager.set_listener_position(camera.position.x, camera.position.y, camera.position.z, camera.front.x, camera.front.y, camera.front.z);
    audio_manager.clean_finished_sounds();

    // Model selection
    Model& model = models[model_names[selected_model]];
    model.draw(camera.get_view_matrix(), projection_matrix);
}

void ViewerScene::display_controls() {
    ImGui::Text("Controls:");
        ImGui::Text("X - Reset camera");
        ImGui::Text("E - switch color");
        ImGui::Text("Q - switch model");
        ImGui::Text("P - take screenshot");
        ImGui::Text("H - play sound");
        ImGui::Text("Scroll - scale model");
        ImGui::Text("Movement:");
        ImGui::Text("Enter Movement Mode - Left Click");
        ImGui::Text("Exit Movement Mode - Right Click");
        ImGui::Text("Movement - WASD + Space + C");
        ImGui::Text("Speed Boost - Left Shift");
}

#pragma region Utils
void ViewerScene::next_model() {
    selected_model = (selected_model + 1) % model_names.size();
}

std::pair<double, double> ViewerScene::get_last_cursor() {
    return { cursor_last_x, cursor_last_y };
}

void ViewerScene::next_color() {
    selected_color = (selected_color + 1) % 3;

    update_shader_color();
}

void ViewerScene::update_shader_color() {
    glm::vec4 shader_color;
    switch (selected_color) {
        case 0: shader_color = glm::vec4(1, 0, 0, 1); break;
        case 1: shader_color = glm::vec4(0, 1, 0, 1); break;
        case 2: shader_color = glm::vec4(0, 0, 1, 1); break;
    }
    shader_library.at("simple_shader")->set_uniform("uniformColor", shader_color);
}
#pragma endregion

#pragma region Listeners
void ViewerScene::on_mouse_button(int button, int action) {

}

void ViewerScene::on_key(int key, int action) {
    if (!this->enabled) return;
    
    if (action != GLFW_PRESS) return;
    switch (key) {
    case GLFW_KEY_E:
        next_color();
        break;
    case GLFW_KEY_X:
        camera.reset(glm::vec3(0, 0, 10));
        break;
    case GLFW_KEY_Q:
        next_model();
        break;
    case GLFW_KEY_H: {
        auto m_pos = models[model_names[selected_model]].get_position();
        audio_manager.play_3D(
            "ping",           // name
            m_pos.x, m_pos.y, m_pos.z // Sound Source Position
        );
    }
    break;
    default:
        break;
    }
}

void ViewerScene::on_mouse_move(double x, double y) {
    if (this->enabled) {
        camera.process_mouse_movement(x - cursor_last_x, (y - cursor_last_y) * -1.0);
    }

    cursor_last_x = x;
    cursor_last_y = y;
}

void ViewerScene::on_scroll(double offset) {
    if (!this->enabled) return;

    fov -= 10 * offset;
    fov = std::clamp(fov, 10.0f, 170.0f); // limit FOV to reasonable values

    update_projection_matrix();
}

void ViewerScene::on_resize(int w, int h) {
    width = w;
    height = h;

    update_projection_matrix();
}
#pragma endregion

#pragma region Transformation
void ViewerScene::update_projection_matrix()
{
    if (height < 1)
        height = 1;   // avoid division by 0

    float ratio = static_cast<float>(width) / height;

    projection_matrix = glm::perspective(
        glm::radians(fov),   // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90� (extra wide) and 30� (quite zoomed in)
        ratio,               // Aspect Ratio. Depends on the size of your window.
        0.1f,                // Near clipping plane. Keep as big as possible, or you'll get precision issues.
        20000.0f             // Far clipping plane. Keep as little as possible.
    );
}
#pragma endregion