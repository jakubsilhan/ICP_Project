#include <iostream>
#include <thread>
#include <optional>

#include "include/runners/GLApp.hpp"
#include "include/render/Drawings.hpp"
#include "render/SyncedTexture.hpp"
#include "include/utils/GlDebugCallback.hpp"
#include "utils/Screenshot.hpp"
#include "scenes/ShooterScene.hpp"

#include <fstream>
#include <nlohmann/json.hpp> // JSON

#include <tinyfiledialogs/tinyfiledialogs.h>

// ImGUI
#include <imgui.h>               // main ImGUI header
#include <imgui_impl_glfw.h>     // GLFW bindings
#include <imgui_impl_opengl3.h>  // OpenGL bindings

GLApp::GLApp()
{
}

bool GLApp::load_config(const std::string& filename) {
    // Load a config json
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open config file: " << filename << std::endl;
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;

        windowWidth = j["window"]["width"].get<int>();
        windowHeight = j["window"]["height"].get<int>();
    }
    catch (std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return false;
    }

    return true;
}

bool GLApp::init() {
    // TODO should probably be separated into multiple init methods
    // GLFW initialization
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Error: Could not initialize GLFW.\n";
        return false;
    }
    // Version specifications
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // Window config
    if (!load_config("resources/config.json")) {
        std::cerr << "Using default window settings.\n";
    }

    // Window creation
    window = glfwCreateWindow(windowWidth, windowHeight, "OpenGL context", NULL, NULL);
    if (!window) {
        std::cerr << "Error: Could not create GLFW window. \n";
        glfwTerminate();
        return false;
    }

    // Worker window context creation
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);  // Hidden window
    trackerWorkerWindow = glfwCreateWindow(1, 1, "", NULL, window);
    if (!trackerWorkerWindow) {
        std::cerr << "Failed to create worker window\n";
        return false;
    }

    // Context must be assigned per thread (and should not be assigned in multiple ones
    glfwMakeContextCurrent(window);

    // Pointer for callbacks
    glfwSetWindowUserPointer(window, this);

    // VSync
    glfwSwapInterval(vsync_on);

    // Requires assigned context
    if (glewInit() != GLEW_OK) {
        std::cerr << "Error: Could not initialize GLEW.\n";
        return false;
    }

    if (GLEW_ARB_debug_output) {
        glDebugMessageCallback(MessageCallback, 0);
        glEnable(GL_DEBUG_OUTPUT);

        std::cout << "GL_DEBUG enabled" << std::endl;
    }
    else {
        std::cout << "GL_DEBUG NOT SUPPORTED!" << std::endl;
    }

    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
    glfwSetCursorPosCallback(window, glfw_cursor_position_callback);
    glfwSetScrollCallback(window, glfw_scroll_callback);

    // Requires initialized glew
    if (!GLEW_ARB_direct_state_access) {
        std::cerr << "Error: DSA (Direct System Access) is not available.\n";
        return false;
    }

    // Init imgui
    if (!init_imgui()) {
        return false;
    }

    // Get version info
    GLint gl_major_version = 0, gl_minor_version = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &gl_major_version);
    glGetIntegerv(GL_MINOR_VERSION, &gl_minor_version);
    gl_version = std::to_string(gl_major_version) + "." + std::to_string(gl_minor_version);
    
    // Get profile info
    GLint gl_profile_mask = 0;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &gl_profile_mask);
    if (gl_profile_mask & GL_CONTEXT_CORE_PROFILE_BIT) {
        gl_profile = "core";
    } else if (gl_profile_mask & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) {
        gl_profile = "compatibility";
    }
    
    // Init tracking
    faceRecognizer.init();
    if (!captureDevice.open(0)) {
        std::cerr << "Error: Could not open camera.\n";
        return false;
    }
    cameraWidth = (int)captureDevice.get(cv::CAP_PROP_FRAME_WIDTH);
    cameraHeight = (int)captureDevice.get(cv::CAP_PROP_FRAME_HEIGHT);
    framePool.init(
        cameraWidth,
        cameraHeight,
        CV_8UC3,
        SyncedTexture::Interpolation::linear_mipmap_linear
    );
    defaultRecognizedData = RecognizedData{
        std::make_unique<SyncedTexture>(),
        std::vector<cv::Point2f>{},
        cv::Point2f{}
    };

    // Init scene
    activeScene = std::make_unique<ShooterScene>(windowWidth, windowHeight);

    return true;
}

bool GLApp::init_imgui()
{

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        std::cerr << "Error: ImGui for GLWF failed to initialize.\n";
        return false;
    }
    if (!ImGui_ImplOpenGL3_Init()) {
        std::cerr << "Error: ImGui for OpenGL3 failed to initialize.\n";
        return false;
    }
    std::cout << "ImGUI version: " << ImGui::GetVersion() << "\n";

    return true;
}

bool GLApp::run() {
    // Title string
    std::ostringstream titleString;
    glm::vec4 shader_color;

    // Culling
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);

    float last_frame_time = glfwGetTime();

    // Set background color
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // Run tracker
    trackthr = std::jthread(&GLApp::trackerThread, this);

    std::optional<RecognizedData> currentRecognizedData;

    while (!glfwWindowShouldClose(window)) {
        // Reinitializations
        titleString.str("");
        titleString.clear();

        // Get recognized data
        if (auto newRecognizedData = deQueue.tryPopFront()) {
            if (currentRecognizedData) framePool.release(std::move(currentRecognizedData->frame));
            currentRecognizedData = std::move(*newRecognizedData);
        }
        const RecognizedData& recognizedData = currentRecognizedData ? *currentRecognizedData : defaultRecognizedData;

        // Process recognized data
        sceneOn = (recognizedData.faces.size() == 1);

        // Prepare imgui render
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        show_crosshair();

        // The info window
        ImGui::SetNextWindowPos(ImVec2(10, 10));
        if (imgui_full) {
            ImGui::SetNextWindowSize(ImVec2(250, 220));
        }
        else {
            ImGui::SetNextWindowSize(ImVec2(250, 150));
        }
        ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::Text("V-Sync: %s", vsync_on ? "ON" : "OFF");
        ImGui::Text("Antialiasing %s", antialiasing_on ? "ON" : "OFF");
        ImGui::Text("FPS: %.1f", FPS_main.get());
        ImGui::Text("GL Version: %s", gl_version.c_str());
        ImGui::Text("GL Profile: %s", gl_profile.c_str());
        ImGui::Text("Controls:");
        ImGui::Text("U - show/hide more info and camera");

        if (imgui_full) {
            ImGui::Text("V - VSync on/off");
            ImGui::Text("T - Antialising on/off");
            ImGui::Text("P - take screenshot");
            ImGui::Text("F11 - Fullscreen/Windowed");
        }
        ImGui::End();
        if (imgui_full) {
            ImGui::SetNextWindowPos(ImVec2(windowWidth-300-10, 10));
            ImGui::SetNextWindowSize(ImVec2(300, 210));
            ImGui::Begin("Scene info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            activeScene->display_controls();
            ImGui::End();

        }

        if (imgui_full) {
            // The camera window
            ImVec2 cameraSize((int)((float)cameraWidth / cameraHeight * 150), 150);
            ImGui::SetNextWindowPos(ImVec2(10, windowHeight - cameraSize[1] - 10));
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::SetNextWindowSize(cameraSize);
            ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_NoDecoration);
            ImGui::Image((ImTextureID)(intptr_t)recognizedData.frame->get_name(), cameraSize, ImVec2(0, 1), ImVec2(1, 0));
            ImGui::End();
            ImGui::PopStyleVar();
        }

        // drawing
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // React to user
        float current_frame_time = glfwGetTime();   // current time in seconds
        float delta_time = current_frame_time - last_frame_time; // lastFrame stored from previous frame
        last_frame_time = current_frame_time;

        if (!sceneOn) {
            delta_time = 0;
        }
        //if (sceneOn && glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
        if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
        activeScene->process_input(window, delta_time);
        }

        // Render scene - TODO revert
        //if (sceneOn) {
        activeScene->update(delta_time);
        activeScene->render();
        //}

        // display imgui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Switch background and foreground buffers (rendering is always done in background first)
        glfwSwapBuffers(window);

        // Check key, mouse events
        glfwPollEvents();

        // FPS counter
        if (FPS_main.is_updated()) {
            if (vsync_on) {
                titleString << "FPS: " << FPS_main.get() << " VSync: On";
            }
            else {
                titleString << "FPS: " << FPS_main.get() << " VSync: Off";
            }
            glfwSetWindowTitle(window, titleString.str().c_str());
        }
        FPS_main.update();
    }

    endedMain = true;

    return true;
}

void GLApp::trackerThread() {
    cv::Mat cvFrame;
    std::unique_ptr<SyncedTexture> frame;
    std::vector<cv::Point2f> faces;
    cv::Point2f red;

    glfwMakeContextCurrent(trackerWorkerWindow);

    while (!endedMain && !endedThread) {
        captureDevice.read(cvFrame);
        if (cvFrame.empty()) {
            std::cerr << "Cam disconnected? End of video?" << std::endl;
            endedThread = true;
            break;
        }

        faces = faceRecognizer.find_face(cvFrame);
        red = redRecognizer.find_red(cvFrame);

        for (auto face : faces) {
            draw_cross_normalized(cvFrame, face, 30, CV_RGB(0, 255, 0));
        }
        draw_cross_normalized(cvFrame, red, 30);

        frame = framePool.acquire();
        frame->fence_wait();
        frame->replace_image(cvFrame);
        frame->fence_sync();

        deQueue.pushBack(RecognizedData{
            std::move(frame),
            faces,
            red
        });

        if (FPS_tracker.is_updated())
            std::cout << "FPS tracker: " << FPS_tracker.get() << std::endl;
        FPS_tracker.update();
    }
}

#pragma region Callbacks
// Callbacks
void GLApp::glfw_error_callback(int error, const char* description)
{
    std::cerr << "GLFW error: " << description << std::endl;
}

void GLApp::glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    auto this_inst = static_cast<GLApp*>(glfwGetWindowUserPointer(window));

    this_inst->windowWidth = width;
    this_inst->windowHeight = height;

    this_inst->activeScene->on_resize(width, height);

    // set viewport
    glViewport(0, 0, width, height);
}

void GLApp::glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    auto this_inst = static_cast<GLApp*>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS) {
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse) {
            switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT: {
                int mode = glfwGetInputMode(window, GLFW_CURSOR);
                if (mode == GLFW_CURSOR_NORMAL) {
                    // we are outside of application, catch the cursor
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    auto [lx, ly] = this_inst->activeScene->get_last_cursor();
                    glfwSetCursorPos(window, lx, ly);
                }
                else {
                    // we are already inside our game: shoot, click, etc.
                    this_inst->activeScene->on_mouse_button(button, action);
                    std::cout << "Bang!\n";
                }
                break;
            }
            case GLFW_MOUSE_BUTTON_RIGHT:
                // release the cursor
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                break;
            default:
                break;
            }
        }
	}
}

void GLApp::glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto this_inst = static_cast<GLApp*>(glfwGetWindowUserPointer(window));
    if ((action == GLFW_PRESS) || (action == GLFW_REPEAT)) {
        switch (key) {
        case GLFW_KEY_ESCAPE: // Exit The App
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_V: // Vsync on/off
            this_inst->vsync_on = !this_inst->vsync_on;
            glfwSwapInterval(this_inst->vsync_on);
            std::cout << "VSync: " << this_inst->vsync_on << "\n";
            break;
        case GLFW_KEY_T: // Antialiasing on/off
            if (this_inst->antialiasing_on) {
                glDisable(GL_MULTISAMPLE);
            }
            else {
                glEnable(GL_MULTISAMPLE);
            }
            this_inst->antialiasing_on = !this_inst->antialiasing_on;
            std::cout << "Antialiasing: " << this_inst->antialiasing_on << "\n";
            break;
        case GLFW_KEY_U: // UI toggle
            this_inst->imgui_full = !this_inst->imgui_full;
            std::cout << "ImGUI: " << this_inst->imgui_full << "\n";
            break;
        case GLFW_KEY_P: // Screenshot
            {
                auto filterPatterns = std::array{ "*.png" }; 
                const char * path = tinyfd_saveFileDialog(
                    "Save screenshot as...",
                    NULL,
                    filterPatterns.size(),
                    filterPatterns.data(),
                    "PNG files"
                );
                if (!path) {
                    std::cout << "Saving screenshot canceled" << std::endl;
                    break;
                }
                if (!makeScreenshot(path, 0, 0, this_inst->windowWidth, this_inst->windowHeight)) {
                    std::cout << "Failed to save screenshot to: " << path << std::endl;
                    break;
                }
                std::cout << "Saved screenshot to: " << path << std::endl;
            }
            break;
        case GLFW_KEY_F11: // Toggle fullscreen
        {
            this_inst->fullscreen = !this_inst->fullscreen;
            if (this_inst->fullscreen) {
                // Save window data
                glfwGetWindowPos(this_inst->window, &this_inst->backupX, &this_inst->backupY);
                glfwGetWindowSize(this_inst->window, &this_inst->backupW, &this_inst->backupH);
                // Fulscreen
                GLFWmonitor* primary = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(primary);
                glfwSetWindowMonitor(this_inst->window, primary,
                    0, 0, mode->width, mode->height,
                    mode->refreshRate);
            }
            else {
                glfwSetWindowMonitor(this_inst->window, nullptr,
                    this_inst->backupX, this_inst->backupY, // position on screen
                    this_inst->backupW,
                    this_inst->backupH,
                    0);
            }
            break;
        }
        default:
                this_inst->activeScene->on_key(key, action);
            break;
        }
    }
}

void GLApp::glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    auto this_inst = static_cast<GLApp*>(glfwGetWindowUserPointer(window));
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {
        this_inst->activeScene->on_scroll(yoffset);
    }
}

void GLApp::glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    auto app = static_cast<GLApp*>(glfwGetWindowUserPointer(window));

    if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
        app->activeScene->on_mouse_move(xpos, ypos);
    }
}


#pragma endregion

#pragma region Imgui
void GLApp::show_crosshair() {
    int windowWidth, windowHeight;
    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    ImDrawList* draw_list = ImGui::GetForegroundDrawList();
    float centerX = windowWidth * 0.5f;
    float centerY = windowHeight * 0.5f;
    float size = 10.0f; // half-length in pixels

    draw_list->AddLine(ImVec2(centerX - size, centerY), ImVec2(centerX + size, centerY), IM_COL32(255, 255, 255, 255), 2.0f); // horizontal
    draw_list->AddLine(ImVec2(centerX, centerY - size), ImVec2(centerX, centerY + size), IM_COL32(255, 255, 255, 255), 2.0f); // vertical
}
#pragma endregion

GLApp::~GLApp()
{
    // Signal threads to end
    endedMain = true;
    endedThread = true;

    // Join threads
    if (trackthr.joinable()) trackthr.join();

    // clean up ImGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // clean up OpenCV
    cv::destroyAllWindows();

    // clean-up GLFW
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
        glfwTerminate();
    }
}
