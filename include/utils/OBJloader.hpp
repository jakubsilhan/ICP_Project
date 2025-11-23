#pragma once

#include <vector>
#include <filesystem>
#include <GL/glew.h>

#include "assets/Vertex.hpp"

bool loadOBJ(const std::filesystem::path& filename,
	         std::vector <Vertex> & vertices,
	         std::vector <GLuint>& indices);
