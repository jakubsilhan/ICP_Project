#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Atlas{
    public:
        std::vector<glm::vec2> map;
        glm::vec2 size;

        inline glm::vec2 get_tile_coords(unsigned int tile, glm::vec2 coords) {
            return (map[tile]+coords)/size;
        }
};
