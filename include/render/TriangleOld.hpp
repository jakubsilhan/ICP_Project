# pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "assets/Vertex.hpp"

class TriangleOld {
public:
	TriangleOld() = default;
	~TriangleOld();
	bool init();
	void draw();
	void set_color(float r, float g, float b, float a);

private:
	GLuint VAO = 0; // Vertex array object ID - configuration (which VBO, which vertex attributes)
	GLuint VBO = 0; // Vertex buffer object ID - vertex data (positions, colors, texture, coords)
	GLuint shader_program = 0; // Shader program ID
	GLint uniform_color_loc = -1; // Location of uniform_Color

	// Uniform = constant variable accross a single draw call (same for all vertices and fragments

	std::vector<Vertex> triangle_vertices = {
		{{0.0f,  0.5f,  0.0f}},
		{{0.5f, -0.5f,  0.0f}},
		{{-0.5f, -0.5f,  0.0f}}
	};

	bool compile_shaders();

};
