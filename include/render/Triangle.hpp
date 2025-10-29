# pragma once

#include <gl/glew.h>

class Triangle {
public:
	Triangle() = default;
	~Triangle();
	bool init();
	void draw();
	void setColor(float r, float g, float b, float a);

private:
	GLuint VAO = 0; // Vertex array object ID - configuration (which VBO, which vertex attributes)
	GLuint VBO = 0; // Vertex buffer object ID - vertex data (positions, colors, texture, coords)
	GLuint shaderProgram = 0; // Shader program ID
	GLint uniformColorLoc = -1; // Location of uniform_Color

	// Uniform = constant variable accross a single draw call (same for all verrtices and fragents

	bool compileShaders();

};