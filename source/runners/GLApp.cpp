#include <thread>

#include "include/runners/GLApp.hpp"
#include "include/render/drawings.hpp"
#include "include/utils/GlDebugCallback.hpp"

#include <fstream>
#include <nlohmann/json.hpp> // JSON

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

    // Init assets
    init_assets();

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

void GLApp::init_assets() {
    shader_library.emplace("simple_shader", std::make_shared<ShaderProgram>(std::filesystem::path("resources/basic_sdr/basic.vert"), std::filesystem::path("resources/basic_sdr/basic.frag")));

    Model triangle_model = Model("resources/triangle.obj", shader_library.at("simple_shader"));
    scene.emplace("triangle_object", std::move(triangle_model));
}

bool GLApp::run() {
    // Title string
    std::ostringstream titleString;
    glm::vec4 shader_color;

    // first update = manual (no event for update arrived yet)
    glfwGetFramebufferSize(window, &width, &height);
    update_projection_matrix();
    glViewport(0, 0, width, height);

    // Set background color
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // set shader
    auto current_shader = shader_library.at("simple_shader");

    while (!glfwWindowShouldClose(window)) {
        // Reinitializations
        titleString.str("");
        titleString.clear();

        // Prepare imgui render
        if (imgui_on) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGui::SetNextWindowPos(ImVec2(10, 10));
            ImGui::SetNextWindowSize(ImVec2(250, 200));

            ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            ImGui::Text("V-Sync: %s", vsync_on ? "ON" : "OFF");
            ImGui::Text("FPS: %.1f", FPS_main.get());
            ImGui::Text("Controls:");
            ImGui::Text("C - switch color");
            ImGui::Text("V - VSync on/off");
            ImGui::Text("D - show/hide info");
            ImGui::End();
        }

        // Set color
        switch (triangleColorIndex) {
            case 0: shader_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); break; // red
            case 1: shader_color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); break; // green
            case 2: shader_color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); break; // blue
        }

        current_shader->setUniform("uniformColor", shader_color);

        // drawing
        glClear(GL_COLOR_BUFFER_BIT);

        //set View matrix = set CAMERA
        glm::mat4 v_m = glm::lookAt(
            glm::vec3(0, 0, 10), // position of camera
            glm::vec3(0, 0, 0),    // where to look
            glm::vec3(0, 1, 0)     // up direction
        );

        current_shader->setUniform("uV_m", v_m);
        current_shader->setUniform("uP_m", projection_matrix);

        for (auto& [name, model] : scene) {
            model.draw();
        }

        // display imgui
        if (imgui_on) {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

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
    return true;
}

#pragma region OpenCV
bool GLApp::init_cv() {
    // Classic tracker initializations
    faceRecognizer.init();

    staticImage = cv::imread("resources/lock.png");
    warningImage = cv::imread("resources/warning.jpg");

    if (!captureDevice.open(0)) {
        std::cerr << "Error: Could not open camera.\n";
        return false;
    }

    return true;
}

bool GLApp::run_cv() {
    cvdispthr = std::jthread(&GLApp::cvdisplayThread, this);
    trackthr = std::jthread(&GLApp::trackerThread, this);
    return true;
}

void GLApp::cvdisplayThread() {
    RecognizedData data;

    do {
        if (endedThread) break;

        if (!deQueue.empty()) {
            data = deQueue.popFront();

            // Display logic
            switch (data.faces.size()) {
            // 1. No face -> static image
            case 0:
                cv::imshow("Scene", staticImage);
                break;

            // 2. One face -> track "some" object (track red)
            case 1:
                draw_cross_normalized(*data.frame, data.faces.front(), 30, CV_RGB(0, 255, 0));
                draw_cross_normalized(*data.frame, data.red, 30);
                cv::imshow("Scene", *data.frame);
                break;

            // 3. More than one face -> display warning
            default:
                cv::imshow("Scene", warningImage);
            }

            framePool.release(std::move(data.frame));
        }

        // Measure and display fps
        if (FPS_cvdisplay.is_updated())
            std::cout << "FPS CV display: " << FPS_main.get() << std::endl;
        FPS_cvdisplay.update();

    } while (cv::waitKey(10) != 27);

    endedThread = true;
}

void GLApp::trackerThread() {
    std::unique_ptr<cv::Mat> frame;
    std::vector<cv::Point2f> faces;
    cv::Point2f red;

    while (!endedMain && !endedThread) {
        frame = framePool.acquire();
        captureDevice.read(*frame);
        if (frame->empty()) {
            std::cerr << "Cam disconnected? End of video?" << std::endl;
            endedThread = true;
            return;
        }

        faces = faceRecognizer.find_face(*frame);
        red = redRecognizer.find_red(*frame);

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
#pragma endregion

#pragma region Callbacks
// Callbacks
void GLApp::glfw_error_callback(int error, const char* description)
{
    std::cerr << "GLFW error: " << description << std::endl;
}

void GLApp::glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    auto this_inst = static_cast<GLApp*>(glfwGetWindowUserPointer(window));
    this_inst->width = width;
    this_inst->height = height;

    // set viewport
    glViewport(0, 0, width, height);
    //now your canvas has [0,0] in bottom left corner, and its size is [width x height] 

    this_inst->update_projection_matrix();
}

void GLApp::glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS) {
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse) {
            switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT: {
                int mode = glfwGetInputMode(window, GLFW_CURSOR);
                if (mode == GLFW_CURSOR_NORMAL) {
                    // we are outside of application, catch the cursor
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }
                else {
                    // we are already inside our game: shoot, click, etc.
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
        case GLFW_KEY_ESCAPE:
            // Exit The App
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_V:
            // Vsync on/off
            this_inst->vsync_on = !this_inst->vsync_on;
            glfwSwapInterval(this_inst->vsync_on);
            std::cout << "VSync: " << this_inst->vsync_on << "\n";
            break;
        case GLFW_KEY_C:
            // Cycle color index
            this_inst->triangleColorIndex = (this_inst->triangleColorIndex + 1) % 3;
            break;
        case GLFW_KEY_D:
            this_inst->imgui_on = !this_inst->imgui_on;
            std::cout << "ImGUI: " << this_inst->imgui_on << "\n";
            break;
        default:
            break;
        }
    }
}

void GLApp::glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    auto this_inst = static_cast<GLApp*>(glfwGetWindowUserPointer(window));
    this_inst->fov -= 10 * yoffset; // yoffset is mostly +1 or -1; one degree difference in fov is not visible
    this_inst->fov = std::clamp(this_inst->fov, 20.0f, 170.0f); // limit FOV to reasonable values...

    this_inst->update_projection_matrix();
}

void GLApp::glfw_cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
}


#pragma endregion

#pragma region Transformation
void GLApp::update_projection_matrix(void)
{
    if (height < 1)
        height = 1;   // avoid division by 0

    float ratio = static_cast<float>(width) / height;

    projection_matrix = glm::perspective(
        glm::radians(fov),   // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
        ratio,               // Aspect Ratio. Depends on the size of your window.
        0.1f,                // Near clipping plane. Keep as big as possible, or you'll get precision issues.
        20000.0f             // Far clipping plane. Keep as little as possible.
    );
}
#pragma endregion

GLApp::~GLApp()
{
    // Signal threads to end
    endedMain = true;
    endedThread = true;

    // Join threads
    if (cvdispthr.joinable()) cvdispthr.join();
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
