#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "render/TriangleOld.hpp"

bool TriangleOld::init() {
    /*
    Initializing values for rendering
    */

    // Compile shaders
    if (!compile_shaders()) {
        return false;
    }

    // VAO - map for reading the data in the gpu (table of contents)
    glCreateVertexArrays(1, &VAO); // Generate a vertex array object and store its id in VAO
    // VBO - raw storage for data in the gpu (content)
    glCreateBuffers(1, &VBO); // Generate a vertex buffer object and store its id in VBO

    // Upload vertex data to GPU using DSA
    glNamedBufferData(
        VBO,                            // Object to fill with data
        triangle_vertices.size() * sizeof(Vertex),       // Size of data
        triangle_vertices.data(),               // Array with vertices
        GL_STATIC_DRAW                  // Data will not change often, optimized for drawing
    );

    // Bind VBO to VAO
    glVertexArrayVertexBuffer(
        VAO,                // Which VAO to configure
        0,                  // binding index (can have multiple vertex buffers per VAO)
        VBO,                // which VBO to attach
        0,                  // offset inside the buffer where vertex data starts
        sizeof(Vertex)   // stride = number of bytes between consecutive vertices
    );

    // Define vertex attribute
    GLint position_attrib_location = glGetAttribLocation(shader_program, "attribute_Position");
    glEnableVertexArrayAttrib(VAO, position_attrib_location); // Enable attribute 0 (layout)
    glVertexArrayAttribFormat(
        VAO,                                        // Which VAO to configure
        position_attrib_location,                   // attribute index (Location in shader)
        3,                                          // number of components (x,y,z)
        GL_FLOAT,                                   // type of each component (float)
        GL_FALSE,                                   // do not normalize
        offsetof(Vertex, position)                  // relative offset in the buffer
    );

    // Connect attribute 0 to binding index 0 of the VAO
    glVertexArrayAttribBinding(VAO, position_attrib_location, 0);

    // Gen uniform Location (color) from shader
    glUseProgram(shader_program); // Activate shader program for querying
    uniform_color_loc = glGetUniformLocation(shader_program, "uniform_Color"); // Get integer handle for uniform_Color from GPU (to be able to access it to change the value)
    if (uniform_color_loc == -1)
        std::cerr << "Warning: uniform_Color not found in shader. \n";

    return true;
}

bool TriangleOld::compile_shaders() {
    /*
    Helper function to compile and link shaders
    */

    // Vertex shader source code (GPU program for each vertex) - pass position for each vertex
    const char* vertex_shader_src = R"(
        #version 460 core
        in vec3 attribute_Position;     
        void main() {
            gl_Position = vec4(attribute_Position, 1.0);      
        }
    )";

    //const char* vertex_shader_src = R"(
    //    #version 460 core                       // OpenGL 4.6 core profile
    //    layout(location = 0) in vec3 aPos;      // Declares a vertex attribute at attribute index 0 in VAO = position
    //    void main() {
    //        gl_Position = vec4(aPos, 1.0);      // Final vertex position in clip space (allowing for perspective)
    //    }
    //)";

    // Fragment shader source code (GPU program for each pixel) - color pixels for only fragments belonging to triangle
    const char* fragmentShaderSrc = R"(
        #version 460 core
        uniform vec4 uniform_Color;  
        out vec4 FragColor;  
        void main() {                       
            FragColor = uniform_Color;
        }
    )";

    //const char* fragmentShaderSrc = R"(
    //    #version 460 core                       // OpenGL 4.6 core profile
    //    uniform vec4 uniform_Color;             // Declare uniform variable for color (set via glProgramUniform4f)
    //    out vec4 FragColor;                     // Declares output color for current pixel
    //    void main() {                       
    //        FragColor = uniform_Color;          // Same color for every pixel
    //    }
    //)";

    // Compile vertex shader
    GLuint vs = glCreateShader(GL_VERTEX_SHADER); // Create vertex shader object
    glShaderSource(vs, 1, &vertex_shader_src, nullptr); // Attach source code to shader
    glCompileShader(vs); // Compile to shader

    // Compile fragment shader
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER); // Create fragment shader object
    glShaderSource(fs, 1, &fragmentShaderSrc, nullptr); // Attach source code to shader
    glCompileShader(fs); // Compile shader

    // Link shaders into a program
    shader_program = glCreateProgram(); // Create shader program object
    glAttachShader(shader_program, vs); // Attach compile vertex shader
    glAttachShader(shader_program, fs); // Attach compile fragment shader
    glLinkProgram(shader_program); // Link shaders into executable GPU program

    // Delete individual shaders after linking
    glDeleteShader(vs);
    glDeleteShader(fs);

    // Check link status
    GLint success;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shader_program, 512, nullptr, infoLog);
        std::cerr << "Shader Program Link Error:\n" << infoLog << "\n";
        return false;
    }

    return true;
}

void TriangleOld::draw() {
    /*
    �ncapsulated rendering
    */
    glUseProgram(shader_program); // Activate shader program
    glBindVertexArray(VAO); // Bind VAO to tell GPU how to read vertices
    glDrawArrays(GL_TRIANGLES, 0, 3); // Draw 3 vertices as one triangle
}

void TriangleOld::set_color(float r, float g, float b, float a) {
    /*
    Assigning color
    */
    if (uniform_color_loc != -1)
        glProgramUniform4f(shader_program, uniform_color_loc, r, g, b, a); // Set the uniform color via our previously retrieved handle
}

TriangleOld::~TriangleOld() {
    if (VBO) glDeleteBuffers(1, &VBO); // Delete GPU buffer
    if (VAO) glDeleteVertexArrays(1, &VAO); // Delete VAO
    if (shader_program) glDeleteProgram(shader_program); // Delete shader program
}
